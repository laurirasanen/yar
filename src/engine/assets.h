#pragma once

#include "../public/iassets.h"
#include "../public/isky.h"

namespace yar
{
class Assets : public IAssets
{
  public:
    void Initialize() override;

    std::shared_ptr<INode> LoadGLTF(const char* path) override;

    std::shared_ptr<ISky> LoadSky(const char* folder) override;
};

}; // namespace yar
