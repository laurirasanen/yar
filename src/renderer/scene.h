#pragma once

#include "node.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace yar
{
struct RenderBatch
{
    std::vector<std::shared_ptr<Node>> Nodes;
};

class Scene
{
  public:
    Scene();
    ~Scene();

    Scene(const Scene&)            = delete;
    Scene(Scene&&)                 = delete;
    Scene& operator=(const Scene&) = delete;
    Scene& operator=(Scene&&)      = delete;

    void Update(const std::vector<std::shared_ptr<Entity>>& entities);

    void Render();

  private:
    void CullNodes();

    void BatchNodes();

    void SortBatches();

    void UpdateDescriptor();

    std::vector<std::shared_ptr<Node>>           m_nodes;
    std::unordered_map<std::string, RenderBatch> m_batches;
};

extern std::shared_ptr<Scene> g_scene;
}; // namespace yar
