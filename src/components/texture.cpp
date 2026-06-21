#define STB_IMAGE_IMPLEMENTATION
#include "texture.h"

#include <climits>

namespace yar
{
Texture::Texture(
    std::shared_ptr<Renderer> renderer,
    std::string               name,
    uint32_t                  size,
    const void*               data
)
{
    if (size > INT_MAX)
    {
        throw std::runtime_error("Texture data too big");
    }

    m_pixels = stbi_load_from_memory(
        static_cast<const stbi_uc*>(data),
        static_cast<int>(size),
        &m_width,
        &m_height,
        &m_channels,
        4
    );

    if (!m_pixels)
    {
        throw std::runtime_error("Failed to load texture");
    }

    m_image = std::make_shared<VulkanImage>(
        renderer,
        static_cast<const void*>(m_pixels),
        static_cast<uint32_t>(m_width),
        static_cast<uint32_t>(m_height),
        Size()
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
