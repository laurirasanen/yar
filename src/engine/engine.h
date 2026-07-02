#pragma once

#include <memory>
#include <semaphore>
#include <stop_token>
#include <thread>

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

#include "../public/iapp.h"
#include "../public/iengine.h"
#include "../public/input.h"
#include "../renderer/renderer.h"

namespace yar
{
class Engine : public IEngine
{
  public:
    Engine();
    ~Engine();

    int Run(std::shared_ptr<IApplication> app) override;

    WindowInput GetFrameInput() override
    {
        return m_frameInput;
    }

    WindowInput GetTickInput() override
    {
        return m_tickInput;
    }

  private:
    void Frame();
    bool Tick();

    void UpdateInput();

    void TickThread(const std::stop_token token);
    void RenderThread(const std::stop_token token);

    std::shared_ptr<InputSettings> m_inputSettings;

    WindowInput m_frameInput;
    WindowInput m_tickInput;

    std::jthread m_tickThread;
    std::jthread m_renderThread;

    std::binary_semaphore m_mainTickSemaphore {0};
    std::binary_semaphore m_threadTickSemaphore {0};

    std::binary_semaphore m_mainFrameSemaphore {0};
    std::binary_semaphore m_threadFrameSemaphore {0};

    std::shared_ptr<IApplication> m_app;
};
} // namespace yar
