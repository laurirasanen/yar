#pragma once

#include "vma.h"

#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan_core.h>

#include <array>
#include <format>
#include <stdexcept>

namespace yar
{
#define VK_CHECK(RESULT, MSG)                                                                      \
    if (RESULT != VK_SUCCESS)                                                                      \
    {                                                                                              \
        throw std::runtime_error(std::format("{} (VkResult: {})", MSG, static_cast<int>(RESULT))); \
    }

#define MAX_OBJECTS 2048

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

struct RenderAttachment
{
    VkImage       Image;
    VkImageView   ImageView;
    VmaAllocation Allocation;
    VkSampler     Sampler;
    uint32_t      Width;
    uint32_t      Height;
};

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
            return VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
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
            return VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT
                   | VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
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
                               .dstStageMask        = GetPipelineStageFlags(newColorLayout),
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
                               .dstStageMask        = GetPipelineStageFlags(newDepthLayout),
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
    dep.dependencyFlags         = 0;
    dep.pImageMemoryBarriers    = barriers.data();
    dep.imageMemoryBarrierCount = barriers.size();

    vkCmdPipelineBarrier2(commandBuffer, &dep);
}

constexpr static void TransitionImageLayout(
    VkCommandBuffer       commandBuffer,
    VkImage               color,
    VkImageLayout         oldColorLayout,
    VkImageLayout         newColorLayout,
    VkPipelineStageFlags2 srcStage,
    VkPipelineStageFlags2 dstStage,
    VkAccessFlags2        srcAccess,
    VkAccessFlags2        dstAccess
)
{
    VkImageSubresourceRange range {};
    range.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel   = 0;
    range.levelCount     = 1;
    range.baseArrayLayer = 0;
    range.layerCount     = 1;

    VkImageMemoryBarrier2 barrier {
        .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext               = nullptr,
        .srcStageMask        = srcStage,
        .srcAccessMask       = srcAccess,
        .dstStageMask        = dstStage,
        .dstAccessMask       = dstAccess,
        .oldLayout           = oldColorLayout,
        .newLayout           = newColorLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image               = color,
        .subresourceRange    = range,
    };

    VkDependencyInfo dep {};
    dep.sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dep.dependencyFlags         = 0;
    dep.pImageMemoryBarriers    = &barrier;
    dep.imageMemoryBarrierCount = 1;

    vkCmdPipelineBarrier2(commandBuffer, &dep);
}

constexpr static uint32_t VkFormatChannels(VkFormat format)
{
    switch (format)
    {
        case VK_FORMAT_R8G8B8_UNORM:
        case VK_FORMAT_R8G8B8_SRGB:
        case VK_FORMAT_R16G16B16_SFLOAT:
        case VK_FORMAT_R32G32B32_SFLOAT:
        {
            return 3;
        }

        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_R16G16B16A16_SFLOAT:
        case VK_FORMAT_R32G32B32A32_SFLOAT:
        {
            return 4;
        }

        default:
        {
            throw std::runtime_error(
                std::format("Unhandled format {}", static_cast<uint32_t>(format))
            );
        }
    }
}

constexpr static VkShaderModuleCreateInfo GetShaderCreateInfo(const void* data, size_t size)
{
    VkShaderModuleCreateInfo createInfo {};
    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = size;
    createInfo.pCode    = static_cast<const uint32_t*>(data);
    return createInfo;
}

constexpr static VkPipelineShaderStageCreateInfo FillShaderStageCreateInfo(
    VkShaderModuleCreateInfo* module,
    VkShaderStageFlagBits     stage
)
{
    VkPipelineShaderStageCreateInfo createInfo {};
    createInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    createInfo.stage  = stage;
    createInfo.module = VK_NULL_HANDLE;
    createInfo.pName  = "main";
    createInfo.pNext  = module;
    return createInfo;
}

static void CreateImage(
    VkImage*          image,
    VmaAllocation*    imageAllocation,
    VkImageType       imageType,
    VkFormat          format,
    VkImageUsageFlags usage,
    uint32_t          width,
    uint32_t          height
)
{
    VkImageCreateInfo createInfo {};
    createInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.imageType     = imageType;
    createInfo.format        = format;
    createInfo.extent.width  = width;
    createInfo.extent.height = height;
    createInfo.extent.depth  = 1;
    createInfo.mipLevels     = 1;
    createInfo.arrayLayers   = 1;
    createInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    createInfo.usage         = usage;
    createInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    createInfo.flags         = 0;

    VmaAllocationCreateInfo allocInfo {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

    vmaCreateImage(g_vma, &createInfo, &allocInfo, image, imageAllocation, nullptr);
}

static void CreateImageSampler(
    const VkDevice       device,
    float                maxSamplerAnisotropy,
    VkSampler*           sampler,
    VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT
)
{
    VkSamplerCreateInfo samplerInfo     = {};
    samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.minFilter               = VK_FILTER_LINEAR;
    samplerInfo.magFilter               = VK_FILTER_LINEAR;
    samplerInfo.addressModeU            = addressMode;
    samplerInfo.addressModeV            = addressMode;
    samplerInfo.addressModeW            = addressMode;
    samplerInfo.anisotropyEnable        = VK_TRUE;
    samplerInfo.maxAnisotropy           = maxSamplerAnisotropy;
    samplerInfo.borderColor             = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable           = VK_FALSE;
    samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias              = 0.0f;
    samplerInfo.minLod                  = 0.0f;
    samplerInfo.maxLod                  = 0.0f;

    VK_CHECK(
        vkCreateSampler(device, &samplerInfo, nullptr, sampler),
        "Failed to create image sampler"
    );
}

static void CreateImageView(
    VkDevice           device,
    VkImage            image,
    VkImageView*       imageView,
    VkImageViewType    viewType,
    VkFormat           format,
    VkImageAspectFlags aspect
)
{
    VkImageViewCreateInfo createInfo {};
    createInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image                           = image;
    createInfo.viewType                        = viewType;
    createInfo.format                          = format;
    createInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask     = aspect;
    createInfo.subresourceRange.baseMipLevel   = 0;
    createInfo.subresourceRange.levelCount     = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount     = 1;

    VK_CHECK(
        vkCreateImageView(device, &createInfo, nullptr, imageView),
        "Failed to create image view"
    );
}
} // namespace yar
