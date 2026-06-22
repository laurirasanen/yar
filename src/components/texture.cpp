#define STB_IMAGE_IMPLEMENTATION
#include "texture.h"

#include "../renderer/renderer.h"
#include "../renderer/vulkan/image.h"

#include <climits>

namespace yar
{
Texture::Texture(
    std::shared_ptr<Renderer> renderer,
    std::string               name,
    uint32_t                  size,
    const void*               data,
    TextureType               type
) :
    m_name(name)
{
    if (size > INT_MAX)
    {
        throw std::runtime_error("Texture data too big");
    }

    if (type == TextureType::UNKNOWN)
    {
        throw std::runtime_error("Unknown texture type");
    }

    m_channels = 4;

    m_pixels = stbi_load_from_memory(
        static_cast<const stbi_uc*>(data),
        static_cast<int>(size),
        &m_width,
        &m_height,
        &m_originalChannels,
        m_channels
    );

    if (!m_pixels)
    {
        throw std::runtime_error("Failed to convert texture");
    }

    if (m_originalChannels != m_channels)
    {
        LOG_DEBUG(
            "Converted texture {} from {} channels to {}",
            name,
            m_originalChannels,
            m_channels
        );
    }

    m_transparent = false;
    if (m_originalChannels == 4)
    {
        for (size_t i = 0; i < size; i += 4)
        {
            if (m_pixels[i] < 255)
            {
                m_transparent = true;
                break;
            }
        }
    }

    m_image = std::make_shared<VulkanImage>(
        renderer,
        static_cast<const void*>(m_pixels),
        static_cast<uint32_t>(m_width),
        static_cast<uint32_t>(m_height),
        static_cast<uint32_t>(m_channels),
        GetSize(),
        type
    );

    stbi_image_free(m_pixels);
    m_pixels = nullptr;
}

Texture::~Texture()
{
    if (m_pixels != nullptr)
    {
        stbi_image_free(m_pixels);
        m_pixels = nullptr;
    }
}
}; // namespace yar
