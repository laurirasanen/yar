#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "../public/renderer/irenderer.h"
#include "../public/resource/texture.h"
#include "../renderer/renderer.h"

#include <ktxvulkan.h>

#include <climits>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <string>

namespace yar
{
Texture::Texture(std::string name, TextureType type) :
    Resource(name),
    m_type(type),
    m_size(0),
    m_data(nullptr)
{
}

Texture::Texture(std::string name, TextureType type, size_t size, void* data) :
    Resource(name),
    m_type(type),
    m_size(size),
    m_data(data)
{
}

bool Texture::DoLoad()
{
    const auto& renderer = static_pointer_cast<Renderer>(g_renderer);
    const auto& device   = renderer->GetDevice();

    std::vector<uint8_t> bytes;
    if (m_data == nullptr)
    {
        bytes  = fs_read_data(GetId().c_str());
        m_size = bytes.size();
        m_data = static_cast<void*>(bytes.data());
    }

    if (m_data == nullptr)
    {
        return false;
    }

    if (m_type == TextureType::TEX_KTX)
    {
        ktxTexture*         kTexture;
        KTX_error_code      result;
        ktxVulkanDeviceInfo vdi;
        m_ktxVulkanTexture = std::make_unique<ktxVulkanTexture>();

        ktxVulkanDeviceInfo_Construct(
            &vdi,
            device.GetVkPhysicalDevice(),
            device.GetVkDevice(),
            device.GetGraphicsQueue(),
            device.GetCommandPool(),
            nullptr
        );

        result = ktxTexture_CreateFromMemory(
            static_cast<const ktx_uint8_t*>(m_data),
            m_size,
            KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
            &kTexture
        );

        if (result != KTX_SUCCESS)
        {
            LOG_ERROR("Failed to create KTX {}: {}", GetId(), static_cast<int>(result));
            return false;
        }

        result = ktxTexture_VkUploadEx(
            kTexture,
            &vdi,
            m_ktxVulkanTexture.get(),
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        if (result != KTX_SUCCESS)
        {
            LOG_ERROR("Failed to upload KTX {}: {}", GetId(), static_cast<int>(result));
            return false;
        }

        ktxTexture_Destroy(kTexture);
        ktxVulkanDeviceInfo_Destruct(&vdi);

        m_image = m_ktxVulkanTexture->image;

        m_format   = m_ktxVulkanTexture->imageFormat;
        m_mips     = m_ktxVulkanTexture->levelCount;
        m_viewType = m_ktxVulkanTexture->viewType;

        return true;
    }
    else
    {
        if (m_size > INT_MAX)
        {
            LOG_ERROR("Texture data too big for stb");
            return false;
        }

        void* pixels = nullptr;
        int   width, height, channels;

        switch (m_type)
        {
            case TextureType::TEX_ALBEDO:
            case TextureType::TEX_ORM:
            case TextureType::TEX_NORMAL:
            case TextureType::TEX_EMISSIVE:
            case TextureType::TEX_IBL_LUT:
            {
                pixels = stbi_load_from_memory(
                    static_cast<const stbi_uc*>(m_data),
                    static_cast<int>(m_size),
                    &width,
                    &height,
                    &channels,
                    0
                );
                break;
            }

            case TextureType::TEX_IBL:
            {
                pixels = stbi_loadf_from_memory(
                    static_cast<const stbi_uc*>(m_data),
                    static_cast<int>(m_size),
                    &width,
                    &height,
                    &channels,
                    0
                );
                break;
            }

            default:
            {
                LOG_ERROR("Unknown texture type {}", static_cast<int>(m_type));
                return false;
            }
        }

        if (pixels == nullptr)
        {
            LOG_ERROR("Failed to convert texture");
            return false;
        }

        m_width    = static_cast<uint32_t>(width);
        m_height   = static_cast<uint32_t>(height);
        m_channels = static_cast<uint32_t>(channels);
        m_format   = VK_FORMAT_UNDEFINED;
        m_mips     = 1;
        m_viewType = VK_IMAGE_VIEW_TYPE_2D;

        switch (m_channels)
        {
            case 4:
            {
                switch (m_type)
                {
                    case TextureType::TEX_ALBEDO:
                    {
                        m_format = VK_FORMAT_R8G8B8A8_SRGB;
                        break;
                    }

                    case TextureType::TEX_ORM:
                    case TextureType::TEX_NORMAL:
                    case TextureType::TEX_EMISSIVE:
                    case TextureType::TEX_IBL_LUT:
                    {
                        m_format = VK_FORMAT_R8G8B8A8_UNORM;
                        break;
                    }

                    case TextureType::TEX_IBL:
                    {
                        m_format = VK_FORMAT_R32G32B32A32_SFLOAT;
                        break;
                    }

                    default:
                    {
                        LOG_ERROR(
                            "Unhandled texture type {}, channels {}",
                            static_cast<int>(m_type),
                            m_channels
                        );
                        return false;
                    }
                }
                break;
            }

            case 3:
            {
                switch (m_type)
                {
                    case TextureType::TEX_ALBEDO:
                    {
                        m_format = VK_FORMAT_R8G8B8_SRGB;
                        break;
                    }

                    case TextureType::TEX_ORM:
                    case TextureType::TEX_NORMAL:
                    case TextureType::TEX_EMISSIVE:
                    case TextureType::TEX_IBL_LUT:
                    {
                        m_format = VK_FORMAT_R8G8B8_UNORM;
                        break;
                    }

                    case TextureType::TEX_IBL:
                    {
                        m_format = VK_FORMAT_R32G32B32_SFLOAT;
                        break;
                    }

                    default:
                    {
                        LOG_ERROR(
                            "Unhandled texture type {}, channels {}",
                            static_cast<int>(m_type),
                            m_channels
                        );
                        return false;
                    }
                }
                break;
            }

            default:
            {
                LOG_ERROR("Unhandled channel count {}", m_channels);
                return false;
            }
        }

        VkImageCreateInfo imageInfo = {};
        imageInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType         = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width      = m_width;
        imageInfo.extent.height     = m_height;
        imageInfo.extent.depth      = 1;
        imageInfo.mipLevels         = m_mips;
        imageInfo.arrayLayers       = 1;
        imageInfo.format            = m_format;
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

        LOG_DEBUG("Created image {}", static_cast<void*>(m_image));

        VkImageSubresourceRange imageRange {};
        imageRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        imageRange.baseMipLevel   = 0;
        imageRange.levelCount     = m_mips;
        imageRange.baseArrayLayer = 0;
        imageRange.layerCount     = 1;

        auto                  commandBuffer = renderer->GetTemporaryCommandBuffer();
        VkImageMemoryBarrier2 barrier {
            .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .pNext               = nullptr,
            .srcStageMask        = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
            .srcAccessMask       = VK_ACCESS_2_NONE,
            .dstStageMask        = VK_PIPELINE_STAGE_TRANSFER_BIT,
            .dstAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT,
            .oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout           = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
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

        // Use a staging buffer to transition pixels to the optimal format.
        auto hostBuffer = std::make_shared<Buffer>(
            renderer->GetDevice().GetVkDevice(),
            BufferType::ImageBuffer,
            Host,
            m_size,
            1
        );
        void* hostData;
        hostBuffer->Map(&hostData);
        std::memcpy(hostData, pixels, m_size);
        hostBuffer->Unmap();
        stbi_image_free(pixels);
        pixels = nullptr;

        VkBufferImageCopy region               = {};
        region.bufferOffset                    = 0;
        region.bufferRowLength                 = 0;
        region.bufferImageHeight               = 0;
        region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel       = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount     = 1;
        region.imageOffset                     = {0, 0, 0};
        region.imageExtent                     = {m_width, m_height, 1};

        vkCmdCopyBufferToImage(
            commandBuffer,
            hostBuffer->GetVkBuffer(),
            m_image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );

        barrier.srcStageMask  = VK_PIPELINE_STAGE_TRANSFER_BIT,
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        barrier.dstStageMask  = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
        barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        barrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,

        vkCmdPipelineBarrier2(commandBuffer, &dep);
        renderer->SubmitTemporaryCommandBuffer(commandBuffer);
    }

    VkImageViewCreateInfo viewInfo           = {};
    viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image                           = m_image;
    viewInfo.viewType                        = m_viewType;
    viewInfo.format                          = m_format;
    viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel   = 0;
    viewInfo.subresourceRange.levelCount     = m_mips;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount     = m_mips;

    VK_CHECK(
        vkCreateImageView(renderer->GetDevice().GetVkDevice(), &viewInfo, nullptr, &m_imageView),
        "Failed to create image view"
    );

    VkSamplerCreateInfo samplerInfo     = {};
    samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.minFilter               = VK_FILTER_LINEAR;
    samplerInfo.magFilter               = VK_FILTER_LINEAR;
    samplerInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable        = VK_TRUE;
    samplerInfo.maxAnisotropy           = device.GetProperties().limits.maxSamplerAnisotropy;
    samplerInfo.borderColor             = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable           = VK_FALSE;
    samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias              = 0.0f;
    samplerInfo.minLod                  = 0.0f;
    samplerInfo.maxLod                  = static_cast<float>(m_mips) - 1.0f;

    VK_CHECK(
        vkCreateSampler(device.GetVkDevice(), &samplerInfo, nullptr, &m_sampler),
        "Failed to create image sampler"
    );

    return true;
}

bool Texture::DoUnload()
{
    const auto& renderer = static_pointer_cast<Renderer>(g_renderer);
    VkDevice    device   = renderer->GetDevice().GetVkDevice();

    vkDestroySampler(device, m_sampler, nullptr);
    vkDestroyImageView(device, m_imageView, nullptr);

    if (m_type == TextureType::TEX_KTX)
    {
        ktxVulkanTexture_Destruct(m_ktxVulkanTexture.get(), device, nullptr);
    }
    else
    {
        vmaDestroyImage(g_vma, m_image, m_vmaAllocation);
    }

    return true;
}
}; // namespace yar
