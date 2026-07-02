#include <functional>
#include <memory>
#include <stop_token>
#include <thread>

#include <imgui.h>

#include "../public/log.h"
#include "../ui/ui.h"
#include "../window/window.h"
#include "../world/world.h"
#include "assets.h"
#include "engine.h"

namespace yar
{
std::shared_ptr<IUI>       g_ui;
std::shared_ptr<IWindow>   g_window;
std::shared_ptr<IWorld>    g_world;
std::shared_ptr<IAssets>   g_assets;
std::shared_ptr<IRenderer> g_renderer;

Engine::Engine()
{
    LOG_INFO("Creating Engine");

    Time::SetStart();

    m_inputSettings = std::make_shared<InputSettings>();
    g_window        = std::make_shared<SDLWindow>(m_inputSettings);

    g_renderer = std::make_shared<Renderer>(static_pointer_cast<SDLWindow>(g_window));

    g_ui = std::make_shared<UI>();

    g_assets = std::make_shared<Assets>();
    g_assets->Initialize();

    g_world = std::make_shared<World>();

    m_frameInput.Clear();
    m_tickInput.Clear();

    g_window->SetMouseGrab(true);

    auto fps = g_window->GetRefreshRate();
    LOG_DEBUG("Setting framerate to {}", fps);
    Time::SetFrameRate(fps);

    m_tickThread   = std::jthread {std::bind_front(&Engine::TickThread, this)};
    m_renderThread = std::jthread {std::bind_front(&Engine::RenderThread, this)};
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
    g_renderer->WaitForIdle();

    m_app.reset();
    g_scene.reset();
    g_world.reset();
    g_ui.reset();
    g_window.reset();
    g_assets.reset();
    g_renderer.reset();
}

int Engine::Run(std::shared_ptr<IApplication> app)
{
    m_mainFrameSemaphore.release();
    m_mainTickSemaphore.release();

    Time::UpdateTickDelta();
    Time::UpdateFrameDelta();

    m_app      = app;
    auto start = m_app->Start();
    if (start != 0)
    {
        return start;
    }

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

    return 0;
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

    m_app->Frame();

    const auto camera = g_renderer->GetCamera();
    camera->HandleInput(m_frameInput);

    if (m_frameInput.wantsResize)
    {
        int width;
        int height;
        g_window->GetFramebufferSize(&width, &height);
        camera->UpdateViewport(width, height);
        g_renderer->Resize();
    }

    g_world->Frame();

    if (m_frameInput.WasPressed(Key::KEY_MOUSE_GRAB))
    {
        g_window->SetMouseGrab(!g_window->IsMouseGrabbed());
        m_frameInput.Clear(true);
    }

    if (m_frameInput.WasPressed(Key::KEY_WINDOW_DEBUG))
    {
        g_ui->ToggleWindow(UIWindow::DEBUG);
        m_frameInput.SetKeyUp(Key::KEY_WINDOW_DEBUG);
    }

    if (m_frameInput.WasPressed(Key::KEY_WINDOW_DEMO))
    {
        g_window->SetMouseGrab(false);
        m_frameInput.Clear();
        g_ui->ToggleWindow(UIWindow::DEMO);
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
    static_pointer_cast<SDLWindow>(g_window)->AggregateInput(m_frameInput);
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

        m_app->Tick();

        g_world->Tick();

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

        if (g_window->IsMinimized())
        {
            Time::StopRender();
            m_mainFrameSemaphore.release();
            continue;
        }

        g_renderer->ResetFrameStats();
        g_renderer->Begin();
        g_renderer->UpdateUniforms();

        g_world->Render();

        g_ui->Render();

        g_renderer->Submit();

        g_renderer->Present();

        Time::StopRender();

        m_mainFrameSemaphore.release();
    }

    LOG_INFO("Exit RenderThread");
}
} // namespace yar
