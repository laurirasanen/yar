#pragma once

#include <memory>
#include <semaphore>
#include <stop_token>
#include <thread>

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

#include "../public/iengine.h"
#include "../renderer/renderer.h"
#include "../ui/ui.h"
#include "../window/input.h"
#include "../window/window.h"
#include "../world/world.h"

namespace yar
{
class Engine : public IEngine
{
  public:
    Engine();
    ~Engine();

    int Run() override;

    std::shared_ptr<IWindow> GetWindow() override
    {
        return m_window;
    }

  private:
    void Frame();
    bool Tick();

    void UpdateInput();

    void TickThread(const std::stop_token token);
    void RenderThread(const std::stop_token token);

    std::shared_ptr<InputSettings> m_inputSettings;
    std::shared_ptr<Window>        m_window;
    std::shared_ptr<Camera>        m_camera;
    std::shared_ptr<Renderer>      m_renderer;
    std::shared_ptr<World>         m_world;
    std::shared_ptr<UI>            m_ui;

    WindowInput m_frameInput;
    WindowInput m_tickInput;

    std::jthread m_tickThread;
    std::jthread m_renderThread;

    std::binary_semaphore m_mainTickSemaphore {0};
    std::binary_semaphore m_threadTickSemaphore {0};

    std::binary_semaphore m_mainFrameSemaphore {0};
    std::binary_semaphore m_threadFrameSemaphore {0};
};
} // namespace yar
