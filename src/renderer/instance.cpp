#include <cstdint>
#include <stdexcept>

#include "../public/log.h"
#include "../public/platform/env.h"
#include "../public/util.h"
#include "common.h"
#include "instance.h"

namespace yar
{
constexpr static VKAPI_ATTR VkBool32 VKAPI_CALL _VkDebugMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* /*pUserData*/
)
{
    if (pCallbackData == nullptr || pCallbackData->pMessage == nullptr)
    {
        return VK_FALSE;
    }

    auto level = LogLevel::Debug;
    switch (messageSeverity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            level = LogLevel::Debug;
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            level = LogLevel::Debug;
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            level = LogLevel::Warn;
            break;
        default:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            level = LogLevel::Error;
            break;
    }

    _LOG(level, "vvl: {}", pCallbackData->pMessage);

    return VK_FALSE;
}

constexpr static VkResult _CreateDebugUtilsMessengerEXT(
    VkInstance                                instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks*              pAllocator,
    VkDebugUtilsMessengerEXT*                 pDebugMessenger
)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

constexpr static void _DestroyDebugUtilsMessengerEXT(
    VkInstance                   instance,
    VkDebugUtilsMessengerEXT     debugMessenger,
    const VkAllocationCallbacks* pAllocator
)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

constexpr static void _BeginDebugUtilsLabelEXT(
    VkInstance                  instance,
    VkCommandBuffer             commandBuffer,
    const VkDebugUtilsLabelEXT* pLabelInfo
)
{
    auto func = (PFN_vkCmdBeginDebugUtilsLabelEXT)
        vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT");
    if (func != nullptr)
    {
        func(commandBuffer, pLabelInfo);
    }
}

constexpr static void _EndDebugUtilsLabelEXT(VkInstance instance, VkCommandBuffer commandBuffer)
{
    auto func = (PFN_vkCmdEndDebugUtilsLabelEXT)
        vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT");
    if (func != nullptr)
    {
        func(commandBuffer);
    }
}

constexpr static void _SetDebugUtilsObjectNameEXT(
    VkInstance                           instance,
    VkDevice                             device,
    const VkDebugUtilsObjectNameInfoEXT* pNameInfo
)
{
    auto func = (PFN_vkSetDebugUtilsObjectNameEXT)
        vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");
    if (func != nullptr)
    {
        func(device, pNameInfo);
    }
}

VulkanInstance::VulkanInstance(std::shared_ptr<SDLWindow> window) : m_window(window)
{
    LOG_DEBUG("Creating VulkanInstance");

    VkApplicationInfo appInfo {};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "yar";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "yar";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_4;

    if (Environment::IsSet("RENDERDOC_CAPFILE"))
    {
        m_enableLayerSettings = false;
    }

    if (m_enableValidationLayers && !ValidationLayersSupported())
    {
        throw std::runtime_error("Validation layers not available");
    }

    auto extensions = GetRequiredExtensions();

    VkInstanceCreateInfo createInfo {};
    createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo        = &appInfo;
    createInfo.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (m_enableValidationLayers)
    {
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
        createInfo.enabledLayerCount   = static_cast<uint32_t>(m_validationLayers.size());
        createInfo.ppEnabledLayerNames = m_validationLayers.data();

        PopulateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = &debugCreateInfo;

        if (m_enableLayerSettings)
        {
            const char*    layer    = "VK_LAYER_KHRONOS_validation";
            const VkBool32 on       = VK_TRUE;
            const VkBool32 off      = VK_FALSE;
            const char*    action[] = {"VK_DBG_LAYER_ACTION_LOG_MSG"};
            const char*    flags[]  = {"info", "warn", "perf", "error", "debug"};

            // clang-format off
            const VkLayerSettingEXT settings[] = {
                {layer, "legacy_detection",            VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &on},
                {layer, "validate_sync",               VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &on},
                {layer, "validate_core",               VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &off},
                {layer, "gpuav_enable",                VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &on},
                {layer, "validate_best_practices",     VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &on},
                {layer, "validate_best_practices_amd", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &on},
                {layer, "debug_action",                VK_LAYER_SETTING_TYPE_STRING_EXT, 1, action},
                {layer, "report_flags",                VK_LAYER_SETTING_TYPE_STRING_EXT, 5, flags}
            };
            // clang-format on

            VkLayerSettingsCreateInfoEXT settingsCreateInfo;
            settingsCreateInfo.sType        = VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT;
            settingsCreateInfo.pNext        = nullptr;
            settingsCreateInfo.settingCount = ARRAY_SIZE(settings);
            settingsCreateInfo.pSettings    = settings;

            debugCreateInfo.pNext = &settingsCreateInfo;
        }

        VK_CHECK(
            vkCreateInstance(&createInfo, nullptr, &m_vkInstance),
            "Failed to create instance"
        );

        SetupDebugMessenger();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext             = nullptr;

        VK_CHECK(
            vkCreateInstance(&createInfo, nullptr, &m_vkInstance),
            "Failed to create instance"
        );
    }

    CreateSurface();
}

VulkanInstance::~VulkanInstance()
{
    LOG_DEBUG("Destroying VulkanInstance");

    vkDestroySurfaceKHR(m_vkInstance, m_vkSurface, nullptr);

    if (m_enableValidationLayers)
    {
        _DestroyDebugUtilsMessengerEXT(m_vkInstance, m_vkDebugMessenger, nullptr);
    }

    vkDestroyInstance(m_vkInstance, nullptr);
}

void VulkanInstance::SetWindow(std::shared_ptr<SDLWindow> window)
{
    m_window = window;
    vkDestroySurfaceKHR(m_vkInstance, m_vkSurface, nullptr);
    CreateSurface();
}

std::vector<const char*> VulkanInstance::GetRequiredExtensions()
{
    unsigned int             extensionCount = 0;
    std::vector<const char*> extensions(extensionCount);

    const char* const* sdlExt = m_window->GetVulkanExtensions(&extensionCount);
    for (unsigned int i = 0; i < extensionCount; i++)
    {
        extensions.push_back(sdlExt[i]);
    }

    if (m_enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        if (m_enableLayerSettings)
        {
            extensions.push_back(VK_EXT_LAYER_SETTINGS_EXTENSION_NAME);
        }
    }

    return extensions;
}

bool VulkanInstance::ValidationLayersSupported()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const auto* layerName : m_validationLayers)
    {
        auto layerFound = false;

        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}

void constexpr VulkanInstance::PopulateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT& createInfo
)
{
    createInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                 | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
                                 | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                 | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                                 | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                 | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = _VkDebugMessengerCallback;
    createInfo.pUserData       = nullptr;
    createInfo.flags           = 0;
    createInfo.pNext           = nullptr;
}

void VulkanInstance::SetupDebugMessenger()
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo {};
    PopulateDebugMessengerCreateInfo(createInfo);

    VK_CHECK(
        _CreateDebugUtilsMessengerEXT(m_vkInstance, &createInfo, nullptr, &m_vkDebugMessenger),
        "Failed to set up debug messenger"
    );
}

void VulkanInstance::CreateSurface()
{
    auto result = m_window->CreateVulkanSurface(m_vkInstance, &m_vkSurface);
    if (!result)
    {
        throw std::runtime_error("Failed to create vulkan surface");
    }
}

void VulkanInstance::BeginDebugLabel(
    VkCommandBuffer commandBuffer,
    const char*     name,
    glm::vec4       color
) const
{
    if (!m_enableValidationLayers)
    {
        return;
    }

    VkDebugUtilsLabelEXT info = {};
    info.sType                = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    info.pLabelName           = name;
    info.color[0]             = color.x;
    info.color[1]             = color.y;
    info.color[2]             = color.z;
    info.color[3]             = color.w;
    _BeginDebugUtilsLabelEXT(m_vkInstance, commandBuffer, &info);
}

void VulkanInstance::EndDebugLabel(VkCommandBuffer commandBuffer) const
{
    if (!m_enableValidationLayers)
    {
        return;
    }

    _EndDebugUtilsLabelEXT(m_vkInstance, commandBuffer);
}

void VulkanInstance::SetDebugName(
    VkDevice     device,
    VkObjectType objectType,
    uint64_t     objectHandle,
    const char*  name
) const
{
    if (!m_enableValidationLayers)
    {
        return;
    }

    VkDebugUtilsObjectNameInfoEXT info = {};
    info.sType                         = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.objectType                    = objectType;
    info.objectHandle                  = objectHandle;
    info.pObjectName                   = name;
    _SetDebugUtilsObjectNameEXT(m_vkInstance, device, &info);
}
} // namespace yar
