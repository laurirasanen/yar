#pragma once

#include "inode.h"
#include "isky.h"

#include <memory>

namespace yar
{
class IAssets
{
  public:
    IAssets()          = default;
    virtual ~IAssets() = default;

    IAssets(const IAssets&)            = delete;
    IAssets(IAssets&&)                 = delete;
    IAssets& operator=(const IAssets&) = delete;
    IAssets& operator=(IAssets&&)      = delete;

    virtual void Initialize() = 0;

    virtual std::shared_ptr<INode> LoadGLTF(const char* path) = 0;

    virtual std::shared_ptr<ISky> LoadSky(const char* folder) = 0;
};

extern std::shared_ptr<IAssets> g_assets;
}; // namespace yar
