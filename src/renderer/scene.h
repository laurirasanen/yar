#pragma once

#include "../public/renderer/irenderer.h"

#include <map>
#include <memory>
#include <vector>

namespace yar
{
struct RenderBatch
{
    std::vector<std::shared_ptr<IRenderNode>> Nodes;
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

    void Update(std::vector<std::shared_ptr<INode>> worldNodes);

    void UpdateDescriptor();

    void Render();

  private:
    void CullNodes();

    void BatchNodes();

    void SortBatches();

    std::vector<std::shared_ptr<IRenderNode>> m_nodes;
    std::map<RenderPipeline, RenderBatch>     m_batches;
};

extern std::shared_ptr<Scene> g_scene;
}; // namespace yar
