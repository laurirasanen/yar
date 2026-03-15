#pragma once

#include <type_traits>
#include <vector>

#include <vulkan/vulkan_core.h>

#include "../data_types.h"

namespace yar
{
template<typename T>
consteval VkVertexInputBindingDescription GetVulkanBindingDescription()
{
    VkVertexInputBindingDescription desc {};
    desc.binding   = 0;
    desc.stride    = sizeof(T);
    desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return desc;
}

template<typename T>
constexpr std::vector<VkVertexInputAttributeDescription> GetVulkanAttributeDescriptions()
{
    std::vector<VkVertexInputAttributeDescription> desc {};

    // TODO: vertex pulling

    desc.push_back({
        .location = 0,
        .binding  = 0,
        .format   = VK_FORMAT_R32G32B32_SFLOAT,
        .offset   = offsetof(T, position),
    });
    desc.push_back({
        .location = 1,
        .binding  = 0,
        .format   = VK_FORMAT_R32G32B32_SFLOAT,
        .offset   = offsetof(T, normal),
    });
    desc.push_back({
        .location = 2,
        .binding  = 0,
        .format   = VK_FORMAT_R32G32_SFLOAT,
        .offset   = offsetof(T, uv),
    });
    desc.push_back({
        .location = 3,
        .binding  = 0,
        .format   = VK_FORMAT_R32G32B32_SFLOAT,
        .offset   = offsetof(T, color),
    });

    return desc;
}
} // namespace yar
