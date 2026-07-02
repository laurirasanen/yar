#define STB_IMAGE_IMPLEMENTATION
#include "texture.h"

#include "../renderer/image.h"
#include "../renderer/renderer.h"

#include "ktxvulkan.h"

#include <climits>
#include <cstdlib>
#include <format>

namespace yar
{
Texture::Texture(
    std::shared_ptr<Renderer> renderer,
    std::string               name,
    uint32_t                  size,
    const void*               data,
    TextureType               type,
    bool                      skipConvert
) :
    ITexture(name),
    m_renderer(renderer),
    m_type(type)
{
    if (type == TextureType::TEX_KTX)
    {
        ktxTexture*         kTexture;
        KTX_error_code      result;
        ktxVulkanDeviceInfo vdi;
        VulkanDevice&       device = renderer->GetDevice();
        m_ktxVulkanTexture         = std::make_shared<ktxVulkanTexture>();

        ktxVulkanDeviceInfo_Construct(
            &vdi,
            device.GetVkPhysicalDevice(),
            device.GetVkDevice(),
            device.GetGraphicsQueue(),
            device.GetCommandPool(),
            nullptr
        );

        result = ktxTexture_CreateFromMemory(
            static_cast<const ktx_uint8_t*>(data),
            size,
            KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
            &kTexture
        );

        if (result != KTX_SUCCESS)
        {
            throw std::runtime_error(
                std::format("Failed to create KTX {}: {}", name, static_cast<int>(result))
            );
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
            throw std::runtime_error(
                std::format("Failed to upload KTX {}: {}", name, static_cast<int>(result))
            );
        }

        ktxTexture_Destroy(kTexture);
        ktxVulkanDeviceInfo_Destruct(&vdi);

        m_image = std::make_shared<VulkanImage>(renderer, m_ktxVulkanTexture);
        LOG_DEBUG("KTX {} levels {}", name, m_ktxVulkanTexture->levelCount);

        return;
    }

    if (size > INT_MAX)
    {
        throw std::runtime_error("Texture data too big");
    }

    switch (type)
    {
        case TextureType::TEX_ALBEDO:
        case TextureType::TEX_ORM:
        case TextureType::TEX_NORMAL:
        case TextureType::TEX_EMISSIVE:
        case TextureType::TEX_IBL_LUT:
        {
            m_elementSize = sizeof(uint8_t);
            break;
        }

        case TextureType::TEX_IBL:
        {
            m_elementSize = sizeof(float);
            break;
        }

        default:
        {
            throw std::runtime_error(
                std::format("Unknown texture type {}", static_cast<int>(type))
            );
        }
    }

    if (skipConvert)
    {
        m_pixels = static_cast<stbi_uc*>(malloc(size));
        std::memcpy(m_pixels, data, size);
        m_channels = 4;
        if (size % static_cast<uint32_t>(m_channels) != 0)
        {
            throw std::runtime_error(
                std::format("bad texture size ({} % {} != 0)", size, m_channels)
            );
        }
        const auto count = size / (m_elementSize * static_cast<uint32_t>(m_channels));
        m_width          = static_cast<int>(sqrt(count));
        m_height         = m_width;
        if (GetSize() != size)
        {
            throw std::runtime_error(std::format("bad texture data ({} != {})", GetSize(), size));
        }
    }
    else
    {
        int originalChannels = 0;
        m_channels           = 4;

        if (m_elementSize == 1)
        {
            m_pixels = stbi_load_from_memory(
                static_cast<const stbi_uc*>(data),
                static_cast<int>(size),
                &m_width,
                &m_height,
                &originalChannels,
                m_channels
            );
        }
        else if (m_elementSize == 4)
        {
            m_pixels = stbi_loadf_from_memory(
                static_cast<const stbi_uc*>(data),
                static_cast<int>(size),
                &m_width,
                &m_height,
                &originalChannels,
                m_channels
            );
        }

        if (originalChannels != m_channels)
        {
            LOG_DEBUG("Converted texture {} channels {} -> {}", name, originalChannels, m_channels);
        }
    }

    if (!m_pixels)
    {
        throw std::runtime_error("Failed to convert texture");
    }

    m_transparent = false;
    if (m_channels == 4)
    {
        if (m_elementSize == 1)
        {
            const auto pixels = static_cast<uint8_t*>(m_pixels);
            for (uint32_t i = 3; i < size; i += 4)
            {
                if (pixels[i] < 255)
                {
                    m_transparent = true;
                    break;
                }
            }
        }
        else if (m_elementSize == 4)
        {
            const auto pixels = static_cast<float*>(m_pixels);
            for (uint32_t i = 3; i < size / sizeof(float); i += 4)
            {
                if (pixels[i] < 1.0f)
                {
                    m_transparent = true;
                    break;
                }
            }
        }

        LOG_DEBUG("Texture {} transparent: {}", name, m_transparent);
    }

    m_image = std::make_shared<VulkanImage>(
        renderer,
        m_pixels,
        static_cast<uint32_t>(m_width),
        static_cast<uint32_t>(m_height),
        static_cast<uint32_t>(m_channels),
        GetSize(),
        type
    );

    if (skipConvert)
    {
        free(m_pixels);
        m_pixels = nullptr;
    }
    else
    {
        stbi_image_free(m_pixels);
        m_pixels = nullptr;
    }
}

Texture::~Texture()
{
    if (m_type == TextureType::TEX_KTX)
    {
        ktxVulkanTexture_Destruct(
            m_ktxVulkanTexture.get(),
            m_renderer->GetDevice().GetVkDevice(),
            nullptr
        );
    }
}
}; // namespace yar
