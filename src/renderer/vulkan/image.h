#pragma once

#include "../../components/texture.h"
#include "buffer.h"
#include "common.h"
#include "vma.h"

#include <vector>
#include <vulkan/vulkan_core.h>

namespace yar
{
class Renderer;

class VulkanImage
{
  public:
    VulkanImage() = delete;

    VulkanImage(
        std::shared_ptr<Renderer> renderer,
        const void*               pixels,
        uint32_t                  width,
        uint32_t                  height,
        uint32_t                  channels,
        size_t                    size,
        TextureType               type
    );

    ~VulkanImage();

    VulkanImage(const VulkanImage&)            = delete;
    VulkanImage(VulkanImage&&)                 = delete;
    VulkanImage& operator=(const VulkanImage&) = delete;
    VulkanImage& operator=(VulkanImage&&)      = delete;

    VkImage GetVkImage()
    {
        return m_image;
    }

    VkImageView GetVkImageView()
    {
        return m_imageView;
    }

    VkSampler GetVkSampler()
    {
        return m_sampler;
    }

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
