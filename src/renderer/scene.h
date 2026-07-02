#pragma once

#include "../public/inode.h"

#include <memory>
#include <vector>

namespace yar
{
class Scene
{
  public:
    Scene();
    ~Scene();

    Scene(const Scene&)            = delete;
    Scene(Scene&&)                 = delete;
    Scene& operator=(const Scene&) = delete;
    Scene& operator=(Scene&&)      = delete;

    void SetNodes(std::vector<std::shared_ptr<INode>> nodes);

    void UpdateDescriptor();

    void Render();

  private:
    void CullNodes();

    void SortNodes();

    std::vector<std::shared_ptr<INode>> m_nodes;
};

extern std::shared_ptr<Scene> g_scene;
}; // namespace yar
