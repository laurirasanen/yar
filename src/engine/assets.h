#pragma once

#include "../public/iassets.h"
#include "../public/isky.h"

namespace yar
{
class Assets : public IAssets
{
  public:

    std::shared_ptr<INode> CreateBox(const glm::vec3& extents, const glm::vec3& color) override;
};

}; // namespace yar
