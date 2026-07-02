#pragma once

#include <cstdint>

namespace yar
{
class IImage
{
  public:
    IImage()          = delete;
    virtual ~IImage() = default;

    IImage(uint32_t width, uint32_t height, uint32_t channels, uint32_t mips) :
        m_width(width),
        m_height(height),
        m_channels(channels),
        m_mips(mips) {};

    IImage(const IImage&)            = delete;
    IImage(IImage&&)                 = delete;
    IImage& operator=(const IImage&) = delete;
    IImage& operator=(IImage&&)      = delete;

    uint32_t GetMips() const
    {
        return m_mips;
    }

  protected:
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_channels;
    uint32_t m_mips;
};
}; // namespace yar
