#pragma once

#include <memory>
#include <vector>

#include "../../Window/Window.h"
#include "VulkanCommon.h"

namespace drive
{
class VulkanInstance
{
  public:
    VulkanInstance(std::shared_ptr<Window> window);
    ~VulkanInstance();

    VulkanInstance(const VulkanInstance&)            = delete;
    VulkanInstance(VulkanInstance&&)                 = delete;
    VulkanInstance& operator=(const VulkanInstance&) = delete;
    VulkanInstance& operator=(VulkanInstance&&)      = delete;

    VkInstance GetVkInstance() const
    {
        return m_vkInstance;
    }

    VkSurfaceKHR GetSurface() const
    {
        return m_vkSurface;
    }

    void GetFramebufferSize(int* width, int* height) const
    {
        m_window->GetFramebufferSize(width, height);
    }

    void SetWindow(std::shared_ptr<Window> window);

  private:
    bool                     ValidationLayersSupported();
    std::vector<const char*> GetRequiredExtensions();
    void constexpr PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void SetupDebugMessenger();
    void CreateSurface();

    std::shared_ptr<Window> m_window;

    VkInstance               m_vkInstance;
    VkDebugUtilsMessengerEXT m_vkDebugMessenger;
    VkSurfaceKHR             m_vkSurface;

    const std::vector<const char*> m_validationLayers = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
    constexpr static bool m_enableValidationLayers = false;
#else
    constexpr static bool m_enableValidationLayers = true;
#endif
};
} // namespace drive
