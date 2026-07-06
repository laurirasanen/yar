#pragma once

#include <optional>

#include <vulkan/vulkan_core.h>

#include "../public/geometry.h"
#include "instance.h"
#include "post_process.h"
#include "vma.h"

namespace yar
{
struct VulkanQueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool IsComplete() const
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct VulkanSwapchainSupportDetails
{
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};

class VulkanDevice
{
  public:
    VulkanDevice(const VulkanInstance& instance);
    ~VulkanDevice();

    VulkanDevice(const VulkanDevice&)            = delete;
    VulkanDevice(VulkanDevice&&)                 = delete;
    VulkanDevice& operator=(const VulkanDevice&) = delete;
    VulkanDevice& operator=(VulkanDevice&&)      = delete;

    void Begin();
    void ResetViewport();
    void SetViewport(Rect rect);
    void PostProcess();
    void BeginUI();
    void EndUI();
    void Submit();
    void Present();

    VkCommandBuffer GetTemporaryCommandBuffer();
    void            SubmitTemporaryCommandBuffer(VkCommandBuffer commandBuffer);

    VkCommandBuffer GetCommandBuffer() const
    {
        return m_vkCommandBuffers[m_currentFrame];
    }

    uint32_t GetCurrentFrame() const
    {
        return m_currentFrame;
    }

    void ResizeFramebuffer()
    {
        m_frameBufferResized = true;
    }

    VkPhysicalDevice GetVkPhysicalDevice() const
    {
        return m_vkPhysicalDevice;
    }

    VkDevice GetVkDevice() const
    {
        return m_vkDevice;
    }

    constexpr VkExtent2D GetSwapchainExtent() const
    {
        return m_vkSwapchainExtent;
    }

    constexpr float GetSwapchainAspect() const
    {
        const auto extent = GetSwapchainExtent();
        return static_cast<float>(extent.width) / static_cast<float>(extent.height);
    }

    VkFormat GetSwapchainImageFormat() const
    {
        return m_vkSwapchainImageFormat;
    }

    VkFormat GetColorFormat() const
    {
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    }

    VkFormat GetDepthFormat() const
    {
        return VK_FORMAT_D32_SFLOAT_S8_UINT;
    }

    VkDescriptorPool GetDescriptorPool() const
    {
        return m_vkDescriptorPool;
    }

    VkDescriptorPool GetImGuiDescriptorPool() const
    {
        return m_vkImGuiDescriptorPool;
    }

    uint32_t GetGraphicsQueueIndex() const
    {
        return m_vkGraphicsFamilyIndex;
    }

    VkQueue GetGraphicsQueue() const
    {
        return m_vkGraphicsQueue;
    }

    VkPhysicalDeviceProperties GetProperties() const
    {
        return m_vkPhysicalDeviceProperties;
    }

    VkCommandPool GetCommandPool() const
    {
        return m_vkCommandPool;
    }

    VkPresentModeKHR GetPresentMode() const
    {
        return m_presentMode;
    }

    uint32_t GetSwapchainImageCount() const
    {
        return m_swapchainImageCount;
    }

  private:
    void SetupPostprocessing();
    void DestroyPostprocessing();

    void RecreateSwapchain();
    void DestroySwapchain();

    void                     PickPhysicalDevice();
    bool                     IsDeviceSuitable(const VkPhysicalDevice device);
    VulkanQueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice device);
    bool                     CheckDeviceExtensionSupport(const VkPhysicalDevice device);

    VulkanSwapchainSupportDetails QuerySwapchainSupport(const VkPhysicalDevice device);
    constexpr VkSurfaceFormatKHR  ChooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR>& available
    );
    constexpr VkPresentModeKHR ChooseSwapPresentMode(
        const std::vector<VkPresentModeKHR>& available
    );
    constexpr VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    void                 CreateSwapchain();

    void CreateLogicalDevice();

    void CreateCommandPool();
    void CreateCommandBuffers();

    void CreateSyncObjects();

    void CreateDescriptorPools();

    const VulkanInstance& m_instance;

    VkPhysicalDevice m_vkPhysicalDevice;
    VkDevice         m_vkDevice;

    VkPhysicalDeviceProperties m_vkPhysicalDeviceProperties;

    uint32_t m_vkGraphicsFamilyIndex;
    uint32_t m_vkPresentFamilyIndex;

    VkQueue m_vkGraphicsQueue;
    VkQueue m_vkPresentQueue;

    VkPresentModeKHR         m_presentMode;
    uint32_t                 m_swapchainImageCount;
    VkSwapchainKHR           m_vkSwapchain;
    std::vector<VkImage>     m_vkSwapchainImages;
    VkFormat                 m_vkSwapchainImageFormat;
    VkExtent2D               m_vkSwapchainExtent;
    std::vector<VkImageView> m_vkSwapchainImageViews;

    RenderAttachment m_colorAttachment;
    RenderAttachment m_depthAttachment;

    VkCommandPool                m_vkCommandPool;
    std::vector<VkCommandBuffer> m_vkCommandBuffers;

    std::vector<VkFence>     m_inFlightFences;
    std::vector<VkSemaphore> m_acquireSemaphores;
    std::vector<VkSemaphore> m_releaseSemaphores;

    VkDescriptorPool m_vkDescriptorPool;
    VkDescriptorPool m_vkImGuiDescriptorPool;

    uint32_t m_swapchainImageIndex;
    uint32_t m_currentFrame;

    bool m_frameBufferResized = false;

    const uint8_t                                m_bloomPassCount = 8;
    std::vector<std::shared_ptr<DownsamplePass>> m_downsamplePasses;
    std::vector<std::shared_ptr<UpsamplePass>>   m_upsamplePasses;
    std::shared_ptr<TonemapPass>                 m_tonemapPass;

    const std::vector<const char*> m_requiredExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
};
} // namespace yar
