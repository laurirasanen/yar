#pragma once

#include <memory>
#include <mutex>

#include "../public/irenderer.h"
#include "../public/iworld.h"
#include "../renderer/renderer.h"
#include "../ui/ui.h"
#include "gltf_node.h"
#include "sky.h"

namespace yar
{
class World : public IWorld
{
  public:
    World();
    ~World();

    World(const World&)            = delete;
    World(World&&)                 = delete;
    World& operator=(const World&) = delete;
    World& operator=(World&&)      = delete;

    void AddNode(std::shared_ptr<INode> node) override;

    void SetSky(std::shared_ptr<ISky> sky) override
    {
        m_sky         = static_pointer_cast<Sky>(sky);
        auto renderer = static_pointer_cast<Renderer>(g_renderer);
        renderer->SetSky(m_sky);
    }

    void Frame() override;
    void Tick() override;
    void Render() override;

  private:
    std::mutex m_worldMutex;

    std::vector<std::shared_ptr<INode>> m_nodes;

    std::shared_ptr<Sky> m_sky;
};
} // namespace yar
