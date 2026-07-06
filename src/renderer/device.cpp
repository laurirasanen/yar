#include <algorithm>
#include <cstdint>
#include <limits>
#include <set>
#include <stdexcept>
#include <vector>

#include <vulkan/vulkan_core.h>

#include "../public/irenderer.h"
#include "../public/log.h"
#include "common.h"
#include "device.h"

namespace yar
{
VulkanDevice::VulkanDevice(const VulkanInstance& instance) :
    m_instance(instance),
    m_swapchainImageIndex(0),
    m_currentFrame(0)
{
    LOG_INFO("Creating VulkanDevice");

    PickPhysicalDevice();
    CreateLogicalDevice();

    CreateVulkanAllocator(m_instance.GetVkInstance(), m_vkPhysicalDevice, m_vkDevice);

    CreateSwapchain();
    CreateCommandPool();
    CreateCommandBuffers();
    CreateSyncObjects();
    CreateDescriptorPools();
}

VulkanDevice::~VulkanDevice()
{
    LOG_INFO("Destroying VulkanDevice");

    vkDeviceWaitIdle(m_vkDevice);

    vkDestroyDescriptorPool(m_vkDevice, m_vkDescriptorPool, nullptr);
    vkDestroyDescriptorPool(m_vkDevice, m_vkImGuiDescriptorPool, nullptr);

    for (auto& fence : m_inFlightFences)
    {
        vkDestroyFence(m_vkDevice, fence, nullptr);
    }
    for (auto& semaphore : m_acquireSemaphores)
    {
        vkDestroySemaphore(m_vkDevice, semaphore, nullptr);
    }

    vkFreeCommandBuffers(
        m_vkDevice,
        m_vkCommandPool,
        static_cast<uint32_t>(m_vkCommandBuffers.size()),
        m_vkCommandBuffers.data()
    );

    vkDestroyCommandPool(m_vkDevice, m_vkCommandPool, nullptr);

    DestroySwapchain();

    DestroyVulkanAllocator();

    vkDestroyDevice(m_vkDevice, nullptr);
}

void VulkanDevice::Begin()
{
    VK_CHECK(
        vkWaitForFences(m_vkDevice, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX),
        "Failed waiting for in flight fence"
    );

    VK_CHECK(
        vkResetFences(m_vkDevice, 1, &m_inFlightFences[m_currentFrame]),
        "Failed to reset in flight fence"
    );

    const auto startAcquire = Time::Now();
    auto       imageResult  = vkAcquireNextImageKHR(
        m_vkDevice,
        m_vkSwapchain,
        UINT64_MAX,
        m_acquireSemaphores[m_currentFrame],
        VK_NULL_HANDLE,
        &m_swapchainImageIndex
    );
    g_renderer->GetRenderStats().AcquireBlockTime = Time::Now() - startAcquire;

    if (m_swapchainImageIndex >= m_swapchainImageCount)
    {
        throw std::runtime_error(
            std::format(
                "Acquired swapchain image index OOB {}/{}",
                m_swapchainImageIndex,
                m_swapchainImageCount - 1
            )
        );
    }

    if (imageResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapchain();
        return;
    }
    else if (imageResult != VK_SUCCESS && imageResult != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to acquire next swapchain image");
    }

    VK_CHECK(
        vkResetCommandBuffer(m_vkCommandBuffers[m_currentFrame], 0),
        "Failed to reset command buffer"
    );

    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VK_CHECK(
        vkBeginCommandBuffer(m_vkCommandBuffers[m_currentFrame], &beginInfo),
        "Failed to begin command buffer"
    );

    if (m_tonemapPass == nullptr)
    {
        SetupPostprocessing();
    }

    TransitionImageLayout(
        m_vkCommandBuffers[m_currentFrame],
        m_colorAttachment.Image,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        m_depthAttachment.Image,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    );

    VkClearValue clearColor {};
    clearColor.color = {
        {0.0f, 0.0f, 0.0f, 1.0f}
    };
    VkClearValue clearDepth {};
    clearDepth.depthStencil = {1.0f, 0};

    VkRenderingAttachmentInfo colorAttachment {};
    colorAttachment.sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.clearValue  = clearColor;
    colorAttachment.loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.imageView   = m_colorAttachment.ImageView;
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.resolveMode = VK_RESOLVE_MODE_NONE;

    VkRenderingAttachmentInfo depthAttachment {};
    depthAttachment.sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depthAttachment.clearValue  = clearDepth;
    depthAttachment.loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.imageView   = m_depthAttachment.ImageView;
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachment.resolveMode = VK_RESOLVE_MODE_NONE;

    VkRect2D renderArea {};
    renderArea.extent = m_vkSwapchainExtent;
    renderArea.offset = {0, 0};

    VkRenderingInfo renderingInfo {};
    renderingInfo.sType                = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea           = renderArea;
    renderingInfo.viewMask             = 0;
    renderingInfo.layerCount           = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments    = &colorAttachment;
    renderingInfo.pDepthAttachment     = &depthAttachment;
    renderingInfo.pStencilAttachment   = nullptr;
    renderingInfo.flags                = 0;

    vkCmdBeginRendering(m_vkCommandBuffers[m_currentFrame], &renderingInfo);

    ResetViewport();
}

void VulkanDevice::ResetViewport()
{
    VkViewport viewport {};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = static_cast<float>(m_vkSwapchainExtent.width);
    viewport.height   = static_cast<float>(m_vkSwapchainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_vkCommandBuffers[m_currentFrame], 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = m_vkSwapchainExtent;
    vkCmdSetScissor(m_vkCommandBuffers[m_currentFrame], 0, 1, &scissor);
}

void VulkanDevice::SetViewport(Rect rect)
{
    VkViewport viewport {};
    viewport.x        = static_cast<float>(rect.offset.x);
    viewport.y        = static_cast<float>(rect.offset.y);
    viewport.width    = static_cast<float>(rect.size.x);
    viewport.height   = static_cast<float>(rect.size.y);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_vkCommandBuffers[m_currentFrame], 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = {rect.offset.x, rect.offset.y};
    scissor.extent = {static_cast<uint32_t>(rect.size.x), static_cast<uint32_t>(rect.size.y)};
    vkCmdSetScissor(m_vkCommandBuffers[m_currentFrame], 0, 1, &scissor);
}

void VulkanDevice::PostProcess()
{
    vkCmdEndRendering(m_vkCommandBuffers[m_currentFrame]);

    TransitionImageLayout(
        m_vkCommandBuffers[m_currentFrame],
        m_colorAttachment.Image,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        m_depthAttachment.Image,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );

    RenderAttachment previousOutput = m_colorAttachment;
    for (const auto& pass : m_downsamplePasses)
    {
        std::vector<RenderAttachment> inputs = {previousOutput};
        pass->SetInputs(inputs, m_currentFrame);
        pass->Render(m_vkCommandBuffers[m_currentFrame], m_currentFrame);
        pass->TransitionOutput();
        previousOutput = pass->GetOutput();
    }

    for (uint8_t i = 0; i < m_bloomPassCount - 1; i++)
    {
        RenderAttachment downsampleOutput =
            m_downsamplePasses[m_bloomPassCount - 2 - i]->GetOutput();
        std::vector<RenderAttachment> inputs = {previousOutput, downsampleOutput};
        const auto&                   pass   = m_upsamplePasses[i];
        pass->SetInputs(inputs, m_currentFrame);
        pass->Render(m_vkCommandBuffers[m_currentFrame], m_currentFrame);
        pass->TransitionOutput();
        previousOutput = pass->GetOutput();
    }

    TransitionImageLayout(
        m_vkCommandBuffers[m_currentFrame],
        m_vkSwapchainImages[m_swapchainImageIndex],
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_NONE,
        VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT
    );

    {
        RenderAttachment swapchainAttachment = {
            .Image      = m_vkSwapchainImages[m_swapchainImageIndex],
            .ImageView  = m_vkSwapchainImageViews[m_swapchainImageIndex],
            .Allocation = nullptr,
            .Sampler    = nullptr,
            .Width      = m_vkSwapchainExtent.width,
            .Height     = m_vkSwapchainExtent.height
        };
        std::vector<RenderAttachment> inputs = {m_colorAttachment, previousOutput};
        m_tonemapPass->SetInputs(inputs, m_currentFrame);
        m_tonemapPass->SetOutput(swapchainAttachment);
        m_tonemapPass->SetLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR);
        m_tonemapPass->Render(m_vkCommandBuffers[m_currentFrame], m_currentFrame);
    }
}

void VulkanDevice::BeginUI()
{
    VkClearValue clearColor {};
    clearColor.color = {
        {0.0f, 0.0f, 0.0f, 1.0f}
    };

    VkRenderingAttachmentInfo colorAttachment {};
    colorAttachment.sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.clearValue  = clearColor;
    colorAttachment.loadOp      = VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.imageView   = m_vkSwapchainImageViews[m_swapchainImageIndex];
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.resolveMode = VK_RESOLVE_MODE_NONE;

    VkRect2D renderArea {};
    renderArea.extent = m_vkSwapchainExtent;
    renderArea.offset = {0, 0};

    VkRenderingInfo renderingInfo {};
    renderingInfo.sType                = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea           = renderArea;
    renderingInfo.viewMask             = 0;
    renderingInfo.layerCount           = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments    = &colorAttachment;
    renderingInfo.pDepthAttachment     = nullptr;
    renderingInfo.pStencilAttachment   = nullptr;
    renderingInfo.flags                = 0;

    vkCmdBeginRendering(m_vkCommandBuffers[m_currentFrame], &renderingInfo);
}

void VulkanDevice::EndUI()
{
    vkCmdEndRendering(m_vkCommandBuffers[m_currentFrame]);
}

void VulkanDevice::Submit()
{
    TransitionImageLayout(
        m_vkCommandBuffers[m_currentFrame],
        m_vkSwapchainImages[m_swapchainImageIndex],
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        VK_ACCESS_2_NONE
    );

    VK_CHECK(
        vkEndCommandBuffer(m_vkCommandBuffers[m_currentFrame]),
        "Failed to end command buffer"
    );

    VkCommandBufferSubmitInfo bufferInfo {};
    bufferInfo.sType         = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    bufferInfo.commandBuffer = m_vkCommandBuffers[m_currentFrame];

    VkSemaphoreSubmitInfo waitInfo {};
    waitInfo.sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    waitInfo.semaphore = m_acquireSemaphores[m_currentFrame];
    waitInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSemaphoreSubmitInfo signalInfo {};
    signalInfo.sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    signalInfo.semaphore = m_releaseSemaphores[m_swapchainImageIndex];

    VkSubmitInfo2 submitInfo {};
    submitInfo.sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submitInfo.pCommandBufferInfos      = &bufferInfo;
    submitInfo.commandBufferInfoCount   = 1;
    submitInfo.pWaitSemaphoreInfos      = &waitInfo;
    submitInfo.waitSemaphoreInfoCount   = 1;
    submitInfo.pSignalSemaphoreInfos    = &signalInfo;
    submitInfo.signalSemaphoreInfoCount = 1;

    VK_CHECK(
        vkQueueSubmit2(m_vkGraphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]),
        "Failed to submit queue"
    );
}

void VulkanDevice::Present()
{
    VkSemaphore    renderSemaphores[] = {m_releaseSemaphores[m_swapchainImageIndex]};
    VkSwapchainKHR swapchains[]       = {m_vkSwapchain};

    VkPresentInfoKHR presentInfo {};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = renderSemaphores;
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = swapchains;
    presentInfo.pImageIndices      = &m_swapchainImageIndex;

    const auto presentStart  = Time::Now();
    auto       presentResult = vkQueuePresentKHR(m_vkPresentQueue, &presentInfo);
    g_renderer->GetRenderStats().PresentBlockTime = Time::Now() - presentStart;

    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR
        || m_frameBufferResized)
    {
        m_frameBufferResized = false;
        RecreateSwapchain();
    }
    else if (presentResult != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present queue");
    }

    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

VkCommandBuffer VulkanDevice::GetTemporaryCommandBuffer()
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool                 = m_vkCommandPool;
    allocInfo.commandBufferCount          = 1;

    VkCommandBuffer commandBuffer;

    VK_CHECK(
        vkAllocateCommandBuffers(m_vkDevice, &allocInfo, &commandBuffer),
        "Failed to allocate temporary command buffer"
    );

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK(
        vkBeginCommandBuffer(commandBuffer, &beginInfo),
        "Failed to begin temporary command buffer"
    );

    return commandBuffer;
}

void VulkanDevice::SubmitTemporaryCommandBuffer(VkCommandBuffer commandBuffer)
{
    VK_CHECK(vkEndCommandBuffer(commandBuffer), "Failed to end temporary command buffer");

    VkCommandBufferSubmitInfo bufferInfo {};
    bufferInfo.sType         = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    bufferInfo.commandBuffer = commandBuffer;

    VkSubmitInfo2 submitInfo {};
    submitInfo.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submitInfo.pCommandBufferInfos    = &bufferInfo;
    submitInfo.commandBufferInfoCount = 1;

    VK_CHECK(
        vkQueueSubmit2(m_vkGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE),
        "Failed to submit temporary command buffer"
    );

    vkQueueWaitIdle(m_vkGraphicsQueue);
    vkFreeCommandBuffers(m_vkDevice, m_vkCommandPool, 1, &commandBuffer);
}

void VulkanDevice::SetupPostprocessing()
{
    for (uint8_t i = 0; i < m_bloomPassCount; i++)
    {
        auto           pass         = std::make_shared<DownsamplePass>(GetColorFormat());
        const uint32_t outputResDiv = 1 << (i + 1);
        pass->SetOutputSamplerMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
        pass->CreateOutput(
            m_vkSwapchainExtent.width / outputResDiv,
            m_vkSwapchainExtent.height / outputResDiv
        );
        m_downsamplePasses.push_back(pass);
    }

    for (uint8_t i = 0; i < m_bloomPassCount - 1; i++)
    {
        auto           pass         = std::make_shared<UpsamplePass>(GetColorFormat());
        const uint32_t outputResDiv = 1 << (m_bloomPassCount - 2 - i);
        pass->SetOutputSamplerMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
        pass->CreateOutput(
            m_vkSwapchainExtent.width / outputResDiv,
            m_vkSwapchainExtent.height / outputResDiv
        );
        m_upsamplePasses.push_back(pass);
    }

    m_tonemapPass = std::make_shared<TonemapPass>(GetSwapchainImageFormat());
}

void VulkanDevice::DestroyPostprocessing()
{
    m_downsamplePasses.clear();
    m_upsamplePasses.clear();
    m_tonemapPass.reset();
}

void VulkanDevice::RecreateSwapchain()
{
    vkDeviceWaitIdle(m_vkDevice);
    // TODO: should pass previous chain to
    // VkSwapchainCreateInfoKHR.oldSwapchain
    DestroySwapchain();
    CreateSwapchain();
    DestroyPostprocessing();
}

void VulkanDevice::DestroySwapchain()
{
    for (auto view : m_vkSwapchainImageViews)
    {
        vkDestroyImageView(m_vkDevice, view, nullptr);
    }
    for (auto& semaphore : m_releaseSemaphores)
    {
        vkDestroySemaphore(m_vkDevice, semaphore, nullptr);
    }

    vkDestroySwapchainKHR(m_vkDevice, m_vkSwapchain, nullptr);

    vkDestroyImageView(m_vkDevice, m_colorAttachment.ImageView, nullptr);
    vkDestroyImageView(m_vkDevice, m_depthAttachment.ImageView, nullptr);
    vmaDestroyImage(g_vma, m_colorAttachment.Image, m_colorAttachment.Allocation);
    vmaDestroyImage(g_vma, m_depthAttachment.Image, m_depthAttachment.Allocation);
}

void VulkanDevice::PickPhysicalDevice()
{
    LOG_DEBUG("Picking physical device");

    m_vkPhysicalDevice   = VK_NULL_HANDLE;
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance.GetVkInstance(), &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("No GPUs found with Vulkan support");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance.GetVkInstance(), &deviceCount, devices.data());

    for (const auto& device : devices)
    {
        if (IsDeviceSuitable(device))
        {
            m_vkPhysicalDevice = device;
            break;
        }
    }

    if (m_vkPhysicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Failed to find a suitable GPU for vulkan");
    }

    vkGetPhysicalDeviceProperties(m_vkPhysicalDevice, &m_vkPhysicalDeviceProperties);
}

bool VulkanDevice::IsDeviceSuitable(const VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties {};
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    LOG_DEBUG("Device: {}", deviceProperties.deviceName);

    auto familyIndices = FindQueueFamilies(device);
    if (!familyIndices.IsComplete())
    {
        return false;
    }

    auto extensionsSupported = CheckDeviceExtensionSupport(device);
    if (!extensionsSupported)
    {
        return false;
    }

    auto swapchainSupport = QuerySwapchainSupport(device);
    auto swapchainAdequate =
        !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
    if (!swapchainAdequate)
    {
        return false;
    }

    return true;
}

VulkanQueueFamilyIndices VulkanDevice::FindQueueFamilies(const VkPhysicalDevice device)
{
    VulkanQueueFamilyIndices familyIndices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    uint32_t i = 0;
    for (const auto& family : queueFamilies)
    {
        if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            familyIndices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_instance.GetSurface(), &presentSupport);

        if (presentSupport)
        {
            familyIndices.presentFamily = i;
        }

        if (familyIndices.IsComplete())
        {
            break;
        }

        i++;
    }

    return familyIndices;
}

bool VulkanDevice::CheckDeviceExtensionSupport(const VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(
        device,
        nullptr,
        &extensionCount,
        availableExtensions.data()
    );

    LOG_DEBUG("Available extensions:");
    for (auto& ext : availableExtensions)
    {
        LOG_DEBUG("{} {}", ext.extensionName, ext.specVersion);
    }

    std::set<std::string> uniqueRequired(m_requiredExtensions.begin(), m_requiredExtensions.end());
    for (const auto& extension : availableExtensions)
    {
        uniqueRequired.erase(extension.extensionName);
    }

    return uniqueRequired.empty();
}

VulkanSwapchainSupportDetails VulkanDevice::QuerySwapchainSupport(const VkPhysicalDevice device)
{
    VulkanSwapchainSupportDetails details {};

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        device,
        m_instance.GetSurface(),
        &details.capabilities
    );

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_instance.GetSurface(), &formatCount, nullptr);
    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            device,
            m_instance.GetSurface(),
            &formatCount,
            details.formats.data()
        );
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device,
        m_instance.GetSurface(),
        &presentModeCount,
        nullptr
    );
    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device,
            m_instance.GetSurface(),
            &presentModeCount,
            details.presentModes.data()
        );
    }

    return details;
}

constexpr VkSurfaceFormatKHR VulkanDevice::ChooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& available
)
{
    for (const auto& fmt : available)
    {
        if (fmt.format == VK_FORMAT_B8G8R8A8_SRGB
            && fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return fmt;
        }
    }

    LOG_ERROR("Could not find suitable swap surface format, using first available");

    return available[0];
}

constexpr VkPresentModeKHR VulkanDevice::ChooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& available
)
{
    for (const auto& mode : available)
    {
        if (mode == VK_PRESENT_MODE_FIFO_RELAXED_KHR)
        {
            return mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

constexpr VkExtent2D VulkanDevice::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }

    int width;
    int height;
    m_instance.GetFramebufferSize(&width, &height);

    VkExtent2D extent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height),
    };
    extent.width = std::clamp(
        extent.width,
        capabilities.minImageExtent.width,
        capabilities.maxImageExtent.width
    );
    extent.height = std::clamp(
        extent.height,
        capabilities.minImageExtent.height,
        capabilities.maxImageExtent.height
    );

    return extent;
}

void VulkanDevice::CreateSwapchain()
{
    auto support          = QuerySwapchainSupport(m_vkPhysicalDevice);
    auto surfaceFormat    = ChooseSwapSurfaceFormat(support.formats);
    m_presentMode         = ChooseSwapPresentMode(support.presentModes);
    auto extent           = ChooseSwapExtent(support.capabilities);
    m_swapchainImageCount = support.capabilities.minImageCount;
    if (m_presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
    {
        m_swapchainImageCount++;
    }

    LOG_DEBUG("Using {}", PresentModeName(m_presentMode));
    LOG_DEBUG("Requesting {} swapchain images", m_swapchainImageCount);

    VkSwapchainCreateInfoKHR createInfo {};
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface          = m_instance.GetSurface();
    createInfo.minImageCount    = m_swapchainImageCount;
    createInfo.imageFormat      = surfaceFormat.format;
    createInfo.imageColorSpace  = surfaceFormat.colorSpace;
    createInfo.imageExtent      = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform     = support.capabilities.currentTransform;
    createInfo.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode      = m_presentMode;
    createInfo.clipped          = VK_TRUE;
    createInfo.oldSwapchain     = VK_NULL_HANDLE;

    auto     families        = FindQueueFamilies(m_vkPhysicalDevice);
    uint32_t familyIndices[] = {families.graphicsFamily.value(), families.presentFamily.value()};

    if (families.graphicsFamily != families.presentFamily)
    {
        createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices   = familyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VK_CHECK(
        vkCreateSwapchainKHR(m_vkDevice, &createInfo, nullptr, &m_vkSwapchain),
        "Failed to create swapchain"
    );

    const auto prevCount = m_swapchainImageCount;
    VK_CHECK(
        vkGetSwapchainImagesKHR(m_vkDevice, m_vkSwapchain, &m_swapchainImageCount, nullptr),
        "Failed to query swapchain image count"
    );
    if (prevCount != m_swapchainImageCount)
    {
        LOG_WARN("Requested {} swapchain images, got {}", prevCount, m_swapchainImageCount);
    }

    m_vkSwapchainImages.resize(m_swapchainImageCount);

    VK_CHECK(
        vkGetSwapchainImagesKHR(
            m_vkDevice,
            m_vkSwapchain,
            &m_swapchainImageCount,
            m_vkSwapchainImages.data()
        ),
        "Failed to get swapchain images"
    );

    m_vkSwapchainImageFormat = surfaceFormat.format;
    m_vkSwapchainExtent      = extent;

    CreateImage(
        &m_colorAttachment.Image,
        &m_colorAttachment.Allocation,
        VK_IMAGE_TYPE_2D,
        GetColorFormat(),
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        extent.width,
        extent.height
    );
    m_instance.SetDebugName(
        m_vkDevice,
        VK_OBJECT_TYPE_IMAGE,
        (uint64_t)m_colorAttachment.Image,
        "offscreen color"
    );
    m_colorAttachment.Width  = extent.width;
    m_colorAttachment.Height = extent.height;

    CreateImage(
        &m_depthAttachment.Image,
        &m_depthAttachment.Allocation,
        VK_IMAGE_TYPE_2D,
        GetDepthFormat(),
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        extent.width,
        extent.height
    );
    m_instance.SetDebugName(
        m_vkDevice,
        VK_OBJECT_TYPE_IMAGE,
        (uint64_t)m_depthAttachment.Image,
        "offscreen depth"
    );
    m_depthAttachment.Width  = extent.width;
    m_depthAttachment.Height = extent.height;

    const float maxSamplerAnisotropy = m_vkPhysicalDeviceProperties.limits.maxSamplerAnisotropy;
    CreateImageSampler(m_vkDevice, maxSamplerAnisotropy, &m_colorAttachment.Sampler);
    CreateImageSampler(m_vkDevice, maxSamplerAnisotropy, &m_depthAttachment.Sampler);

    m_vkSwapchainImageViews.resize(m_vkSwapchainImages.size());
    m_releaseSemaphores.resize(m_vkSwapchainImages.size());

    VkSemaphoreCreateInfo semaphoreInfo {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (size_t i = 0; i < m_vkSwapchainImages.size(); i++)
    {
        CreateImageView(
            m_vkDevice,
            m_vkSwapchainImages[i],
            &m_vkSwapchainImageViews[i],
            VK_IMAGE_VIEW_TYPE_2D,
            m_vkSwapchainImageFormat,
            VK_IMAGE_ASPECT_COLOR_BIT
        );
        VK_CHECK(
            vkCreateSemaphore(m_vkDevice, &semaphoreInfo, nullptr, &m_releaseSemaphores[i]),
            "Failed to create acquire semaphore"
        );
    }

    CreateImageView(
        m_vkDevice,
        m_colorAttachment.Image,
        &m_colorAttachment.ImageView,
        VK_IMAGE_VIEW_TYPE_2D,
        GetColorFormat(),
        VK_IMAGE_ASPECT_COLOR_BIT
    );
    m_instance.SetDebugName(
        m_vkDevice,
        VK_OBJECT_TYPE_IMAGE_VIEW,
        (uint64_t)m_colorAttachment.ImageView,
        "offscreen color view"
    );

    CreateImageView(
        m_vkDevice,
        m_depthAttachment.Image,
        &m_depthAttachment.ImageView,
        VK_IMAGE_VIEW_TYPE_2D,
        GetDepthFormat(),
        VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT
    );
    m_instance.SetDebugName(
        m_vkDevice,
        VK_OBJECT_TYPE_IMAGE_VIEW,
        (uint64_t)m_depthAttachment.ImageView,
        "offscreen depth view"
    );
}

void VulkanDevice::CreateLogicalDevice()
{
    LOG_DEBUG("Creating logical device");

    VulkanQueueFamilyIndices familyIndices = FindQueueFamilies(m_vkPhysicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t>                   uniqueQueueFamilies = {
        familyIndices.graphicsFamily.value(),
        familyIndices.presentFamily.value()
    };

    auto queuePriority = 1.0f;
    for (uint32_t familyIndex : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo {};
        queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = familyIndex;
        queueCreateInfo.queueCount       = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // 1.0
    VkPhysicalDeviceFeatures2 deviceFeatures {};
    deviceFeatures.features.samplerAnisotropy = VK_TRUE;
    deviceFeatures.sType                      = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

    // 1.1
    VkPhysicalDeviceVulkan11Features vk11Features {};
    vk11Features.sType                = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    vk11Features.shaderDrawParameters = VK_TRUE;
    vk11Features.pNext                = &deviceFeatures;

    // 1.2
    VkPhysicalDeviceVulkan12Features vk12Features {};
    vk12Features.sType                  = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    vk12Features.bufferDeviceAddress    = VK_TRUE;
    vk12Features.runtimeDescriptorArray = VK_TRUE;
    vk12Features.shaderInt8             = VK_TRUE;
    vk12Features.storagePushConstant8   = VK_TRUE;
    vk12Features.pNext                  = &vk11Features;

    // 1.3
    VkPhysicalDeviceVulkan13Features vk13Features {};
    vk13Features.sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    vk13Features.dynamicRendering = VK_TRUE;
    vk13Features.synchronization2 = VK_TRUE;
    vk13Features.pNext            = &vk12Features;

    // 1.4
    VkPhysicalDeviceVulkan14Features vk14Features {};
    vk14Features.sType        = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;
    vk14Features.maintenance5 = VK_TRUE;
    vk14Features.pNext        = &vk13Features;

    VkDeviceCreateInfo deviceCreateInfo {};
    deviceCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos       = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pNext                   = &vk14Features;
    deviceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(m_requiredExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = m_requiredExtensions.data();

    VK_CHECK(
        vkCreateDevice(m_vkPhysicalDevice, &deviceCreateInfo, nullptr, &m_vkDevice),
        "Failed to create logical Vulkan device"
    );

    m_vkGraphicsFamilyIndex = familyIndices.graphicsFamily.value();
    m_vkPresentFamilyIndex  = familyIndices.presentFamily.value();
    vkGetDeviceQueue(m_vkDevice, m_vkGraphicsFamilyIndex, 0, &m_vkGraphicsQueue);
    vkGetDeviceQueue(m_vkDevice, m_vkPresentFamilyIndex, 0, &m_vkPresentQueue);
}

void VulkanDevice::CreateCommandPool()
{
    auto familyIndices = FindQueueFamilies(m_vkPhysicalDevice);

    VkCommandPoolCreateInfo poolInfo {};
    poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = familyIndices.graphicsFamily.value();

    VK_CHECK(
        vkCreateCommandPool(m_vkDevice, &poolInfo, nullptr, &m_vkCommandPool),
        "Failed to create command pool"
    );
}

void VulkanDevice::CreateCommandBuffers()
{
    m_vkCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo {};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool        = m_vkCommandPool;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    VK_CHECK(
        vkAllocateCommandBuffers(m_vkDevice, &allocInfo, m_vkCommandBuffers.data()),
        "Failed to allocate command buffers"
    );
}

void VulkanDevice::CreateSyncObjects()
{
    m_acquireSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VK_CHECK(
            vkCreateSemaphore(m_vkDevice, &semaphoreInfo, nullptr, &m_acquireSemaphores[i]),
            "Failed to create acquire semaphore"
        );
        VK_CHECK(
            vkCreateFence(m_vkDevice, &fenceInfo, nullptr, &m_inFlightFences[i]),
            "Failed to create in flight fence"
        );
    }
}

void VulkanDevice::CreateDescriptorPools()
{
    VkDescriptorPoolSize uboPoolSize {};
    uboPoolSize.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    uboPoolSize.descriptorCount = MAX_FRAMES_IN_FLIGHT;

    VkDescriptorPoolSize imagePoolSize {};
    imagePoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    // TODO this sucks
    const uint32_t texturesPerObject = 4;
    const uint32_t globalTextures    = 64;
    imagePoolSize.descriptorCount =
        MAX_FRAMES_IN_FLIGHT * (MAX_OBJECTS * texturesPerObject + globalTextures);

    std::array<VkDescriptorPoolSize, 2> poolSizes = {uboPoolSize, imagePoolSize};

    VkDescriptorPoolCreateInfo poolInfo {};
    poolInfo.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount         = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes            = poolSizes.data();
    const uint32_t postProcessSets = 32; // TODO this sucks
    poolInfo.maxSets               = MAX_FRAMES_IN_FLIGHT * (1 + postProcessSets);

    VK_CHECK(
        vkCreateDescriptorPool(m_vkDevice, &poolInfo, nullptr, &m_vkDescriptorPool),
        "Failed to create descriptor pool"
    );

    VkDescriptorPoolSize imGuiPoolSize {};
    imGuiPoolSize.type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    imGuiPoolSize.descriptorCount = MAX(2, MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo imGuiPoolInfo {};
    imGuiPoolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    imGuiPoolInfo.poolSizeCount = 1;
    imGuiPoolInfo.pPoolSizes    = &imGuiPoolSize;
    imGuiPoolInfo.maxSets       = MAX(2, MAX_FRAMES_IN_FLIGHT);
    imGuiPoolInfo.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    VK_CHECK(
        vkCreateDescriptorPool(m_vkDevice, &imGuiPoolInfo, nullptr, &m_vkImGuiDescriptorPool),
        "Failed to create ImGui descriptor pool"
    );
}
} // namespace yar
