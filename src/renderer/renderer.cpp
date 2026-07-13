#include <memory>

#include <glm/ext/matrix_transform.hpp>
#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan_core.h>

#include "../public/log.h"
#include "renderer.h"
#include "scene.h"

namespace yar
{
std::shared_ptr<Scene>          g_scene;
std::shared_ptr<ShaderCompiler> g_shaderCompiler;

Renderer::Renderer(std::shared_ptr<SDLWindow> window) :
    IRenderer(),
    m_instance(window),
    m_device(m_instance)
{
    LOG_INFO("Creating Renderer");
}

Renderer::~Renderer()
{
    LOG_INFO("Destroying Renderer");

    vkDeviceWaitIdle(m_device.GetVkDevice());

    m_pipelines.clear();

    g_scene.reset();

    m_descriptorSet.reset();

    m_frameBuffers.clear();
}

void Renderer::SetWindow(std::shared_ptr<SDLWindow> window)
{
    LOG_INFO("Setting window");
    m_instance.SetWindow(window);
    Resize();
}

void Renderer::Resize()
{
    LOG_INFO("Resizing");
    m_device.ResizeFramebuffer();
}

float Renderer::GetAspect()
{
    return m_device.GetSwapchainAspect();
}

void Renderer::Setup()
{
    m_shaderGlobalBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    m_shaderGlobalData.resize(MAX_FRAMES_IN_FLIGHT);
    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_shaderGlobalBuffers[i] = std::make_shared<Buffer>(
            m_device.GetVkDevice(),
            ShaderDataBuffer,
            SecretThirdOption,
            sizeof(ShaderGlobalData),
            1
        );
        m_shaderGlobalData[i] = std::make_shared<ShaderGlobalData>();
    }

    m_descriptorSet = std::make_shared<DescriptorSet>(MAX_FRAMES_IN_FLIGHT);
    m_descriptorSet->Alloc();

    g_shaderCompiler = std::make_shared<ShaderCompiler>();

    g_scene = std::make_shared<Scene>();
}

void Renderer::Begin()
{
    const auto startTime = Time::Now();
    m_device.Begin();
    m_renderStats.SetupTime = Time::Now() - startTime;
}

void Renderer::PostProcess()
{
    const auto startTime = Time::Now();
    m_device.PostProcess();
    m_renderStats.PostProcessTime = Time::Now() - startTime;
}

void Renderer::Submit()
{
    const auto startTime = Time::Now();
    m_device.Submit();
    m_frameBuffers.clear();
    m_renderStats.SubmitTime = Time::Now() - startTime;
}

void Renderer::Present()
{
    const auto startTime = Time::Now();
    m_device.Present();
    m_renderStats.PresentTime = Time::Now() - startTime;
}

void Renderer::UpdateUniforms()
{
    auto currentFrame = m_device.GetCurrentFrame();

    if (m_camera != nullptr)
    {
        int width, height;
        g_window->GetFramebufferSize(&width, &height);
        m_camera->UpdateViewport(width, height);
        m_camera->UpdateMatrices();
        m_camera->UpdateShaderData(m_shaderGlobalData[currentFrame].get());
    }

    std::memcpy(
        m_shaderGlobalBuffers[currentFrame]->GetAllocationInfo().pMappedData,
        m_shaderGlobalData[currentFrame].get(),
        sizeof(ShaderGlobalData)
    );
}

void Renderer::WaitForIdle()
{
    vkDeviceWaitIdle(m_device.GetVkDevice());
}

void Renderer::GetImGuiInfo(VulkanImGuiCreationInfo& info)
{
    info.vkColor         = m_device.GetSwapchainImageFormat();
    VkFormat depthFormat = m_device.GetDepthFormat();

    info.vkPipeline                         = {};
    info.vkPipeline.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    info.vkPipeline.pNext                   = VK_NULL_HANDLE;
    info.vkPipeline.colorAttachmentCount    = 1;
    info.vkPipeline.pColorAttachmentFormats = &info.vkColor;
    info.vkPipeline.depthAttachmentFormat   = depthFormat;
    info.vkPipeline.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    info.imPipeline                             = {};
    info.imPipeline.RenderPass                  = VK_NULL_HANDLE;
    info.imPipeline.Subpass                     = 0;
    info.imPipeline.MSAASamples                 = VK_SAMPLE_COUNT_1_BIT;
    info.imPipeline.ExtraDynamicStates          = {};
    info.imPipeline.PipelineRenderingCreateInfo = info.vkPipeline;

    info.imInit                            = {};
    info.imInit.ApiVersion                 = VK_API_VERSION_1_4;
    info.imInit.Instance                   = m_instance.GetVkInstance();
    info.imInit.PhysicalDevice             = m_device.GetVkPhysicalDevice();
    info.imInit.Device                     = m_device.GetVkDevice();
    info.imInit.QueueFamily                = m_device.GetGraphicsQueueIndex();
    info.imInit.Queue                      = m_device.GetGraphicsQueue();
    info.imInit.DescriptorPool             = m_device.GetImGuiDescriptorPool();
    info.imInit.DescriptorPoolSize         = 0;
    info.imInit.MinImageCount              = 2;
    info.imInit.ImageCount                 = MAX(2, MAX_FRAMES_IN_FLIGHT);
    info.imInit.PipelineCache              = VK_NULL_HANDLE; // TODO
    info.imInit.PipelineInfoMain           = info.imPipeline;
    info.imInit.UseDynamicRendering        = true;
    info.imInit.Allocator                  = nullptr; // TODO vma
    info.imInit.CheckVkResultFn            = ImGuiVkCheck;
    info.imInit.MinAllocationSize          = 1024 * 1024;
    info.imInit.CustomShaderVertCreateInfo = {};
    info.imInit.CustomShaderFragCreateInfo = {};
}
} // namespace yar
