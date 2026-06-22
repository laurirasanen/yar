#include <functional>
#include <memory>
#include <stop_token>
#include <thread>

#include <imgui.h>

#include "engine.h"
#include "log.h"
#include "time.h"
#include "window/window.h"

namespace yar
{
Engine::Engine()
{
    LOG_INFO("Creating Engine");

    Time::SetStart();

    m_inputSettings = std::make_shared<InputSettings>();
    m_window        = std::make_shared<Window>(m_inputSettings);
    m_camera        = std::make_shared<NoclipCamera>();
    m_camera->transform.SetPosition({0.0f, -0.8f, 0.15f});
    m_camera->transform.SetEulerRotation({-10.0f, 0.0f, 0.0f});

    m_renderer = std::make_shared<Renderer>(m_window);

    m_ui = std::make_shared<UI>(m_window, m_renderer, m_camera);

    m_world = std::make_shared<World>(m_renderer, m_ui);

    m_frameInput.Clear();
    m_tickInput.Clear();

    m_window->SetMouseGrab(true);

    m_tickThread   = std::jthread {std::bind_front(&Engine::TickThread, this)};
    m_renderThread = std::jthread {std::bind_front(&Engine::RenderThread, this)};

    auto fps = m_window->GetRefreshRate();
    LOG_DEBUG("Setting framerate to {}", fps);
    Time::SetFrameRate(fps);

    m_mainFrameSemaphore.release();
    m_mainTickSemaphore.release();

    Time::UpdateTickDelta();
    Time::UpdateFrameDelta();

    while (true)
    {
        if (Time::TimeForEngineTick())
        {
            if (!Tick())
            {
                LOG_INFO("Engine::Tick exit");
                break;
            }
        }

        if (Time::TimeForEngineFrame())
        {
            Frame();
        }

        std::this_thread::yield();
    }
}

Engine::~Engine()
{
    LOG_INFO("Destroying Engine");

    LOG_DEBUG("Requesting TickThread stop");
    m_tickThread.request_stop();
    m_threadTickSemaphore.release();
    m_tickThread.join();

    LOG_DEBUG("Requesting RenderThread stop");
    m_renderThread.request_stop();
    m_threadFrameSemaphore.release();
    m_renderThread.join();

    LOG_DEBUG("Waiting for renderer idle");
    m_renderer->WaitForIdle();
}

void Engine::Frame()
{
    auto acquired = m_mainFrameSemaphore.try_acquire();
    if (!acquired)
    {
        return;
    }

    Time::UpdateFrameDelta();

    UpdateInput();

    m_camera->HandleInput(m_frameInput);

    if (m_frameInput.wantsResize)
    {
        int width;
        int height;
        m_window->GetFramebufferSize(&width, &height);
        m_camera->UpdateViewport(width, height);
        m_renderer->Resize();
    }

    m_world->Frame();

    if (m_frameInput.HasKey(Key::KEY_MOUSE_GRAB))
    {
        m_window->SetMouseGrab(!m_window->IsMouseGrabbed());
        m_frameInput.Clear(true);
    }

    if (m_frameInput.HasKey(Key::KEY_WINDOW_DEBUG))
    {
        m_ui->ToggleWindow(UIWindow::DEBUG);
        m_frameInput.KeyUp(Key::KEY_WINDOW_DEBUG);
    }

    if (m_frameInput.HasKey(Key::KEY_WINDOW_DEMO))
    {
        m_window->SetMouseGrab(false);
        m_frameInput.Clear();
        m_ui->ToggleWindow(UIWindow::DEMO);
    }

    m_frameInput.Clear();

    m_threadFrameSemaphore.release();
}

bool Engine::Tick()
{
    UpdateInput();

    if (m_tickInput.wantsQuit)
    {
        return false;
    }

    m_tickInput.Clear();

    auto acquired = m_mainTickSemaphore.try_acquire();
    if (!acquired)
    {
        return true;
    }

    const auto delta         = Time::TimeSinceEngineTick();
    const auto slowThreshold = Time::TickInterval * 2.0;
    if (delta > slowThreshold)
    {
        LOG_WARN("Tick thread ran slow: {:.2f}ms", 1000 * Time::TimeSinceEngineTick());
    }

    Time::UpdateTickDelta();

    m_threadTickSemaphore.release();

    return true;
}

void Engine::UpdateInput()
{
    m_window->AggregateInput(m_frameInput);
    m_tickInput.Aggregate(m_frameInput);
}

void Engine::TickThread(const std::stop_token token)
{
    LOG_INFO("Enter TickThread");

    while (!token.stop_requested())
    {
        m_threadTickSemaphore.acquire();
        if (token.stop_requested())
        {
            break;
        }

        m_world->Tick();

        m_mainTickSemaphore.release();
    }

    LOG_INFO("Exit TickThread");
}

void Engine::RenderThread(const std::stop_token token)
{
    LOG_INFO("Enter RenderThread");

    while (!token.stop_requested())
    {
        m_threadFrameSemaphore.acquire();
        if (token.stop_requested())
        {
            break;
        }

        Time::StartRender();

        if (m_window->IsMinimized())
        {
            Time::StopRender();
            m_mainFrameSemaphore.release();
            continue;
        }

        m_renderer->ResetFrameStats();
        m_renderer->Begin();
        m_renderer->UpdateUniforms(m_camera);

        m_world->Render(m_camera);

        m_ui->Render();

        m_renderer->Submit();

        m_renderer->Present();

        Time::StopRender();

        m_mainFrameSemaphore.release();
    }

    LOG_INFO("Exit RenderThread");
}
} // namespace yar
