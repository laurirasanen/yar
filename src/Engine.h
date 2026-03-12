#pragma once

#include <memory>
#include <semaphore>
#include <stop_token>
#include <thread>

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

#include "Renderer/Renderer.h"
#include "UI/UI.h"
#include "Window/Input.h"
#include "Window/Window.h"
#include "World/World.h"

namespace drive
{
class Engine
{
  public:
    Engine(RendererType rendererType);
    ~Engine();

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
    std::unique_ptr<UI>            m_ui;

    WindowInput m_frameInput;
    WindowInput m_tickInput;

    std::jthread m_tickThread;
    std::jthread m_renderThread;

    std::binary_semaphore m_mainTickSemaphore {0};
    std::binary_semaphore m_threadTickSemaphore {0};

    std::binary_semaphore m_mainFrameSemaphore {0};
    std::binary_semaphore m_threadFrameSemaphore {0};
};
} // namespace drive
