#pragma once

#include "iimage.h"

#include <memory>

namespace yar
{
class ITexture
{
  public:
    ITexture()          = delete;
    virtual ~ITexture() = default;

    ITexture(std::string name) : m_name(name) {};

    ITexture(const ITexture&)            = delete;
    ITexture(ITexture&&)                 = delete;
    ITexture& operator=(const ITexture&) = delete;
    ITexture& operator=(ITexture&&)      = delete;

    virtual std::shared_ptr<IImage> GetImage() = 0;

    std::string Name() const
    {
        return m_name;
    }

  private:
    std::string m_name;
};
}; // namespace yar
