#pragma once

#include "../public/itexture.h"

#include "stb_image.h"

#include <cstdint>
#include <memory>
#include <string>

class ktxVulkanTexture;

namespace yar
{
class Renderer;
class VulkanImage;

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

class Texture : public ITexture
{
  public:
    Texture() = delete;
    Texture(
        std::shared_ptr<Renderer> renderer,
        std::string               name,
        uint32_t                  size,
        const void*               data,
        TextureType               type,
        bool                      skipConvert = false
    );
    ~Texture();

    Texture(const Texture&)            = delete;
    Texture(Texture&&)                 = delete;
    Texture& operator=(const Texture&) = delete;
    Texture& operator=(Texture&&)      = delete;

    size_t GetSize() const
    {
        return static_cast<size_t>(m_width * m_height * m_channels) * m_elementSize;
    }

    int GetChannels() const
    {
        return m_channels;
    }

    std::shared_ptr<IImage> GetImage() override
    {
        return static_pointer_cast<IImage>(m_image);
    }

    bool HasTransparency()
    {
        return m_transparent;
    }

  private:
    std::shared_ptr<Renderer> m_renderer;

    TextureType m_type;

    int      m_width;
    int      m_height;
    int      m_channels;
    uint32_t m_elementSize;

    void* m_pixels;

    bool m_transparent;

    std::shared_ptr<VulkanImage> m_image;

    std::shared_ptr<ktxVulkanTexture> m_ktxVulkanTexture;
};
}; // namespace yar
