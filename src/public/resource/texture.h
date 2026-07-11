#pragma once

#include "resource.h"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <memory>
#include <string>

class ktxVulkanTexture;

namespace yar
{
enum TextureType
{
    TEX_UNKNOWN,
    TEX_ALBEDO,
    TEX_ORM,
    TEX_NORMAL,
    TEX_EMISSIVE,
    TEX_IBL,
    TEX_IBL_LUT,
    TEX_KTX,
};

class Texture : public Resource
{
  public:
    Texture() = delete;
    explicit Texture(std::string name, TextureType type);
    explicit Texture(std::string name, TextureType type, size_t size, const void* data);
    ~Texture();

    Texture(const Texture&)            = delete;
    Texture(Texture&&)                 = delete;
    Texture& operator=(const Texture&) = delete;
    Texture& operator=(Texture&&)      = delete;

    uint32_t GetWidth() const
    {
        return m_width;
    }

    uint32_t GetHeight() const
    {
        return m_height;
    }

    uint32_t GetChannels() const
    {
        return m_channels;
    }

    uint32_t GetMips() const
    {
        return m_mips;
    }

    VkImage GetImage() const
    {
        return m_image;
    }

    VkImageView GetImageView() const
    {
        return m_imageView;
    }

    VkSampler GetSampler() const
    {
        return m_sampler;
    }

  protected:
    bool DoLoad() override;
    bool DoUnload() override;

  private:
    TextureType m_type;
    size_t      m_size;
    const void* m_data;

    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_channels;
    uint32_t m_mips;

    VkImage           m_image;
    VkImageView       m_imageView;
    VkSampler         m_sampler;
    VmaAllocation     m_vmaAllocation;
    VmaAllocationInfo m_vmaAllocationInfo;

    VkImageViewType m_viewType;
    VkFormat        m_format;

    std::unique_ptr<ktxVulkanTexture> m_ktxVulkanTexture;
};
}; // namespace yar
