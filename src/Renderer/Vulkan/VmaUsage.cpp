#include "vk_mem_alloc.h"
#define VMA_IMPLEMENTATION
#define VMA_VULKAN_VERSION 1003000 // Vulkan 1.3
#include "VmaUsage.h"
#include "VulkanCommon.h"

namespace drive
{
VmaAllocator g_vma;

void CreateVulkanAllocator(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device)
{
    VmaAllocatorCreateInfo createInfo {};
    createInfo.instance         = instance;
    createInfo.physicalDevice   = physicalDevice;
    createInfo.device           = device;
    createInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    VK_CHECK(vmaCreateAllocator(&createInfo, &g_vma), "Failed to crate vulkan allocator");
}

void DestroyVulkanAllocator()
{
    vmaDestroyAllocator(g_vma);
}

VmaTotalStatistics GetVulkanAllocatorTotalStatistics()
{
    VmaTotalStatistics stats {};
    vmaCalculateStatistics(g_vma, &stats);
    return stats;
}
} // namespace drive
