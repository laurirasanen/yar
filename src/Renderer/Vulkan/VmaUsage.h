#pragma once

#include <vk_mem_alloc.h>

namespace yar
{
extern void CreateVulkanAllocator(
    VkInstance       instance,
    VkPhysicalDevice physicalDevice,
    VkDevice         device
);

extern void DestroyVulkanAllocator();

extern VmaTotalStatistics GetVulkanAllocatorTotalStatistics();

extern VmaAllocator g_vma;
} // namespace yar
