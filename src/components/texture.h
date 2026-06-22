#pragma once

#include "stb_image.h"

#include <cstdint>
#include <memory>
#include <string>

namespace yar
{
class Renderer;
class VulkanImage;

enum TextureType
{
    UNKNOWN,
    SRGB
};

class Texture
{
  public:
    Texture() = delete;
    Texture(
        std::shared_ptr<Renderer> renderer,
        std::string               name,
        uint32_t                  size,
        const void*               data,
        TextureType               type
    );
    ~Texture();

    Texture(const Texture&)            = delete;
    Texture(Texture&&)                 = delete;
    Texture& operator=(const Texture&) = delete;
    Texture& operator=(Texture&&)      = delete;

    std::string Name() const
    {
        return m_name;
    }

    size_t GetSize() const
    {
        return static_cast<size_t>(m_width * m_height * m_channels);
    }

    int GetChannels() const
    {
        return m_channels;
    }

    std::shared_ptr<VulkanImage> GetImage()
    {
        return m_image;
    }

    bool HasTransparency()
    {
        return m_originalChannels == 4;
    }

  private:
    std::string m_name;

    int m_width;
    int m_height;
    int m_channels;
    int m_originalChannels;

    stbi_uc* m_pixels;

    std::shared_ptr<VulkanImage> m_image;
};
}; // namespace yar
