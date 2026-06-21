#pragma once

#include "buffer.h"
#include "common.h"
#include "src/renderer/renderer.h"
#include "vma.h"

#include <vector>
#include <vulkan/vulkan_core.h>

namespace yar
{
class VulkanImage
{
  public:
    VulkanImage() = delete;

    VulkanImage(
        std::shared_ptr<Renderer> renderer,
        const void*               pixels,
        uint32_t                  width,
        uint32_t                  height,
        size_t                    size
    ) :
        m_renderer(renderer),
        m_width(width),
        m_height(height)
    {
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType         = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width      = width;
        imageInfo.extent.height     = height;
        imageInfo.extent.depth      = 1;
        imageInfo.mipLevels         = 1;
        imageInfo.arrayLayers       = 1;
        imageInfo.format            = VK_FORMAT_R8G8B8A8_SRGB;
        imageInfo.tiling            = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage             = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.sharingMode       = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples           = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags             = 0;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.flags                   = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        allocInfo.usage                   = VMA_MEMORY_USAGE_AUTO;

        VK_CHECK(
            vmaCreateImage(
                g_vma,
                &imageInfo,
                &allocInfo,
                &m_image,
                &m_vmaAllocation,
                &m_vmaAllocationInfo
            ),
            "Failed to create image"
        );

        VK_CHECK(
            vmaCopyMemoryToAllocation(g_vma, pixels, m_vmaAllocation, 0, size),
            "Failed to copy image data"
        );

        auto commandBuffer = renderer->GetTemporaryCommandBuffer();

        VkImageSubresourceRange imageRange {};
        imageRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        imageRange.baseMipLevel   = 0;
        imageRange.levelCount     = 1;
        imageRange.baseArrayLayer = 0;
        imageRange.layerCount     = 1;

        VkImageMemoryBarrier2 barrier {
            .sType         = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .pNext         = nullptr,
            .srcStageMask  = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
            .srcAccessMask = VK_ACCESS_2_NONE,
            .dstStageMask  = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
            .oldLayout     = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image               = m_image,
            .subresourceRange    = imageRange,
        };

        VkDependencyInfo dep {};
        dep.sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dep.dependencyFlags         = 0;
        dep.pImageMemoryBarriers    = &barrier;
        dep.imageMemoryBarrierCount = 1;

        vkCmdPipelineBarrier2(commandBuffer, &dep);

        renderer->SubmitTemporaryCommandBuffer(commandBuffer);

        VkImageViewCreateInfo viewInfo           = {};
        viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image                           = m_image;
        viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format                          = VK_FORMAT_R8G8B8A8_SRGB;
        viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel   = 0;
        viewInfo.subresourceRange.levelCount     = 1;
        viewInfo.subresourceRange.baseArrayLayer = 1;
        viewInfo.subresourceRange.layerCount     = 1;

        VK_CHECK(
            vkCreateImageView(
                renderer->GetDevice().GetVkDevice(),
                &viewInfo,
                nullptr,
                &m_imageView
            ),
            "Failed to create image view"
        );

        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType               = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.minFilter           = VK_FILTER_LINEAR;
        samplerInfo.magFilter           = VK_FILTER_LINEAR;
        samplerInfo.addressModeU        = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV        = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW        = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable    = VK_TRUE;
        samplerInfo.maxAnisotropy =
            m_renderer->GetDevice().GetProperties().limits.maxSamplerAnisotropy;
        samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable           = VK_FALSE;
        samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias              = 0.0f;
        samplerInfo.minLod                  = 0.0f;
        samplerInfo.maxLod                  = 0.0f;

        VK_CHECK(
            vkCreateSampler(
                m_renderer->GetDevice().GetVkDevice(),
                &samplerInfo,
                nullptr,
                &m_sampler
            ),
            "Failed to create image sampler"
        );
    }

    ~VulkanImage()
    {
        vkDestroySampler(m_renderer->GetDevice().GetVkDevice(), m_sampler, nullptr);
        vkDestroyImageView(m_renderer->GetDevice().GetVkDevice(), m_imageView, nullptr);
        vmaDestroyImage(g_vma, m_image, m_vmaAllocation);
    }

    VulkanImage(const VulkanImage&)            = delete;
    VulkanImage(VulkanImage&&)                 = delete;
    VulkanImage& operator=(const VulkanImage&) = delete;
    VulkanImage& operator=(VulkanImage&&)      = delete;

  private:
    std::shared_ptr<Renderer> m_renderer;

    uint32_t m_width;
    uint32_t m_height;

    VkImage           m_image;
    VkImageView       m_imageView;
    VkSampler         m_sampler;
    VmaAllocation     m_vmaAllocation;
    VmaAllocationInfo m_vmaAllocationInfo;
};
}; // namespace yar
