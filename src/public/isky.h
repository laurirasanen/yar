#pragma once

#include "itexture.h"

#include <memory>

namespace yar
{
class ISky
{
  public:
    ISky()          = default;
    virtual ~ISky() = default;

    ISky(const ISky&)            = delete;
    ISky(ISky&&)                 = delete;
    ISky& operator=(const ISky&) = delete;
    ISky& operator=(ISky&&)      = delete;

    virtual std::shared_ptr<ITexture> GetColor()    = 0;
    virtual std::shared_ptr<ITexture> GetLUT()      = 0;
    virtual std::shared_ptr<ITexture> GetDiffuse()  = 0;
    virtual std::shared_ptr<ITexture> GetSpecular() = 0;

    virtual void Render() = 0;
};
}; // namespace yar
