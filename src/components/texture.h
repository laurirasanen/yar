#pragma once

#include "../renderer/renderer.h"
#include "../renderer/vulkan/image.h"

#include "stb_image.h"

#include <memory>
#include <string>

namespace yar
{
class Texture
{
  public:
    Texture() = delete;
    Texture(std::shared_ptr<Renderer> renderer, std::string name, uint32_t size, const void* data);
    ~Texture();

    Texture(const Texture&)            = delete;
    Texture(Texture&&)                 = delete;
    Texture& operator=(const Texture&) = delete;
    Texture& operator=(Texture&&)      = delete;

    std::string Name() const
    {
        return m_name;
    }

    size_t Size() const
    {
        return static_cast<size_t>(m_width * m_height * m_channels);
    }

  private:
    std::string m_name;

    int m_width;
    int m_height;
    int m_channels;

    stbi_uc* m_pixels;

    std::shared_ptr<VulkanImage> m_image;
};
}; // namespace yar
