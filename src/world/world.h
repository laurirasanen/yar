#pragma once

#include <memory>
#include <mutex>

#include "../components/model.h"
#include "../renderer/renderer.h"
#include "../ui/ui.h"

namespace yar
{
class World
{
  public:
    World() = delete;
    World(std::shared_ptr<Renderer> renderer, std::shared_ptr<UI> ui);
    ~World();

    World(const World&)            = delete;
    World(World&&)                 = delete;
    World& operator=(const World&) = delete;
    World& operator=(World&&)      = delete;

    void Frame();
    void Tick();
    void Render(std::shared_ptr<Camera> camera);

  private:
    std::mutex m_worldMutex;

    std::shared_ptr<Renderer> m_renderer;
    std::shared_ptr<UI>       m_ui;

    std::shared_ptr<Buffer> m_testPlaneVertexBuffer;
    std::shared_ptr<Buffer> m_testPlaneIndexBuffer;

    std::vector<std::shared_ptr<Model>> m_models;
};
} // namespace yar
