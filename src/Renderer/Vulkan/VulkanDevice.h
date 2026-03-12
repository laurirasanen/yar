#pragma once

#include <optional>

#include "../../Components/Rect.h"
#include "VmaUsage.h"
#include "VulkanInstance.h"

namespace drive
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
    VulkanDevice(const VulkanInstance& instance, uint32_t maxFramesInFlight);
    ~VulkanDevice();

    VulkanDevice(const VulkanDevice&)            = delete;
    VulkanDevice(VulkanDevice&&)                 = delete;
    VulkanDevice& operator=(const VulkanDevice&) = delete;
    VulkanDevice& operator=(VulkanDevice&&)      = delete;

    void Begin();
    void ResetViewport();
    void SetViewport(Rect rect);
    void Submit();
    void Present();
    void WaitForGraphicsIdle();

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

    VkFormat GetDepthFormat() const
    {
        return VK_FORMAT_D32_SFLOAT;
    }

    VkDescriptorPool GetUboDescriptorPool() const
    {
        return m_vkUboDescriptorPool;
    }

    VkDescriptorPool GetImGuiDescriptorPool() const
    {
        return m_vkImGuiDescriptorPool;
    }

    uint32_t GetGraphicsQueueIndex() const
    {
        return m_vkGraphicsQueueIndex;
    }

    VkQueue GetGraphicsQueue() const
    {
        return m_vkGraphicsQueue;
    }

  private:
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
    constexpr VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available
    );
    constexpr VkExtent2D       ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    void                       CreateSwapchain();

    void CreateImage(
        VkImage*          image,
        VmaAllocation*    imageAllocation,
        VkImageType       imageType,
        VkFormat          format,
        VkImageUsageFlags usage,
        uint32_t          width,
        uint32_t          height
    );
    void CreateImageView(
        VkImage            image,
        VkImageView*       imageView,
        VkImageViewType    viewType,
        VkFormat           format,
        VkImageAspectFlags aspect
    );
    void CreateImageViews();

    void CreateLogicalDevice();

    void CreateCommandPool();
    void CreateCommandBuffers();

    void CreateSyncObjects();

    void CreateDescriptorPools();

    const VulkanInstance& m_instance;

    VkPhysicalDevice m_vkPhysicalDevice;
    VkDevice         m_vkDevice;

    uint32_t m_vkGraphicsQueueIndex;
    uint32_t m_vkPresentQueueIndex;

    VkQueue m_vkGraphicsQueue;
    VkQueue m_vkPresentQueue;

    VkSwapchainKHR           m_vkSwapchain;
    std::vector<VkImage>     m_vkSwapchainImages;
    VkFormat                 m_vkSwapchainImageFormat;
    VkExtent2D               m_vkSwapchainExtent;
    std::vector<VkImageView> m_vkSwapchainImageViews;

    VkImage       m_vkDepthImage;
    VmaAllocation m_vmaDepthAllocation;
    VkImageView   m_vkDepthImageView;

    VkCommandPool                m_vkCommandPool;
    std::vector<VkCommandBuffer> m_vkCommandBuffers;

    std::vector<VkSemaphore> m_vkImageSemaphores;
    std::vector<VkSemaphore> m_vkRenderSemaphores;
    std::vector<VkFence>     m_vkInFlightFences;

    VkDescriptorPool m_vkUboDescriptorPool;
    VkDescriptorPool m_vkImGuiDescriptorPool;

    uint32_t       m_currentImageIndex;
    uint32_t       m_currentFrame = 0;
    const uint32_t m_maxFramesInFlight;

    bool m_frameBufferResized = false;

    const std::vector<const char*> m_requiredExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
    };
};
} // namespace drive
