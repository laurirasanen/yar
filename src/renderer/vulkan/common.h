#pragma once

#include <array>
#include <stdexcept>

#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan_core.h>

namespace yar
{
#define VK_CHECK(RESULT, MSG)          \
    if (RESULT != VK_SUCCESS)          \
    {                                  \
        throw std::runtime_error(MSG); \
    }

constexpr static void ImGuiVkCheck(VkResult result)
{
    VK_CHECK(result, "ImGuiVkCheck failed");
}

// Hold color format so it doesn't get dropped from stack
struct VulkanImGuiCreationInfo
{
    VkFormat                      vkColor;
    VkPipelineRenderingCreateInfo vkPipeline;
    ImGui_ImplVulkan_PipelineInfo imPipeline;
    ImGui_ImplVulkan_InitInfo     imInit;
};

// TODO: relevant for 1.4 / synchronization2?
constexpr static VkPipelineStageFlags2 GetPipelineStageFlags(const VkImageLayout imageLayout)
{
    switch (imageLayout)
    {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            return VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            return VK_PIPELINE_STAGE_2_HOST_BIT;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            return VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            return VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            return VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT
                   | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
        case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
            return VK_PIPELINE_STAGE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            return VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            return VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
        default:
            throw std::runtime_error("Unhandled VkImageLayout to VkPipelineStageFlags2 conversion");
    }
}

constexpr static VkAccessFlags2 GetAccessFlags(const VkImageLayout imageLayout)
{
    switch (imageLayout)
    {
        case VK_IMAGE_LAYOUT_UNDEFINED:
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            return 0;
        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            return VK_ACCESS_2_HOST_WRITE_BIT;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            return VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            return VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT
                   | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
            return VK_ACCESS_2_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            return VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            return VK_ACCESS_2_TRANSFER_READ_BIT;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            return VK_ACCESS_2_TRANSFER_WRITE_BIT;
        default:
            throw std::runtime_error("Unhandled VkImageLayout to VkAccessFlags2 conversion");
    }
}

constexpr static void TransitionImageLayout(
    VkCommandBuffer commandBuffer,
    VkImage         color,
    VkImageLayout   oldColorLayout,
    VkImageLayout   newColorLayout,
    VkImage         depth,
    VkImageLayout   oldDepthLayout,
    VkImageLayout   newDepthLayout
)
{
    VkImageSubresourceRange colorRange {};
    colorRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    colorRange.baseMipLevel   = 0;
    colorRange.levelCount     = 1;
    colorRange.baseArrayLayer = 0;
    colorRange.layerCount     = 1;

    VkImageSubresourceRange depthRange {};
    depthRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    depthRange.baseMipLevel   = 0;
    depthRange.levelCount     = 1;
    depthRange.baseArrayLayer = 0;
    depthRange.layerCount     = 1;

    std::array<VkImageMemoryBarrier2, 2> barriers {
        VkImageMemoryBarrier2 {
                               .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                               .pNext               = nullptr,
                               .srcStageMask        = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                               .srcAccessMask       = GetAccessFlags(oldColorLayout),
                               .dstStageMask        = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                               .dstAccessMask       = GetAccessFlags(newColorLayout),
                               .oldLayout           = oldColorLayout,
                               .newLayout           = newColorLayout,
                               .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                               .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                               .image               = color,
                               .subresourceRange    = colorRange,
                               },
        VkImageMemoryBarrier2 {
                               .sType        = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                               .pNext        = nullptr,
                               .srcStageMask = VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                               .srcAccessMask =
                               GetAccessFlags(oldDepthLayout) | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                               .dstStageMask        = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
                               .dstAccessMask       = GetAccessFlags(newDepthLayout),
                               .oldLayout           = oldDepthLayout,
                               .newLayout           = newDepthLayout,
                               .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                               .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                               .image               = depth,
                               .subresourceRange    = depthRange,
                               }
    };

    VkDependencyInfo dep {};
    dep.sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dep.dependencyFlags         = 0; // TODO: the docs on this suck
    dep.pImageMemoryBarriers    = barriers.data();
    dep.imageMemoryBarrierCount = barriers.size();

    vkCmdPipelineBarrier2(commandBuffer, &dep);
}

constexpr static void TransitionImageLayout(
    VkCommandBuffer commandBuffer,
    VkImage         color,
    VkImageLayout   oldColorLayout,
    VkImageLayout   newColorLayout
)
{
    VkImageSubresourceRange colorRange {};
    colorRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    colorRange.baseMipLevel   = 0;
    colorRange.levelCount     = 1;
    colorRange.baseArrayLayer = 0;
    colorRange.layerCount     = 1;

    VkImageMemoryBarrier2 barrier {
        .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext               = nullptr,
        .srcStageMask        = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask       = GetAccessFlags(oldColorLayout),
        .dstStageMask        = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask       = GetAccessFlags(newColorLayout),
        .oldLayout           = oldColorLayout,
        .newLayout           = newColorLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image               = color,
        .subresourceRange    = colorRange,
    };

    VkDependencyInfo dep {};
    dep.sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dep.dependencyFlags         = 0; // TODO: the docs on this suck
    dep.pImageMemoryBarriers    = &barrier;
    dep.imageMemoryBarrierCount = 1;

    vkCmdPipelineBarrier2(commandBuffer, &dep);
}
} // namespace yar
