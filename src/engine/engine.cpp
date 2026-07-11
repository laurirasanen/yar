#include <functional>
#include <memory>
#include <stop_token>
#include <thread>

#include <imgui.h>

#include "../public/log.h"
#include "../public/resource/resource.h"
#include "../public/time_util.h"
#include "../ui/ui.h"
#include "../window/window.h"
#include "../world/world.h"
#include "engine.h"

namespace yar
{
std::shared_ptr<IUI>             g_ui;
std::shared_ptr<IWindow>         g_window;
std::shared_ptr<IWorld>          g_world;
std::shared_ptr<ResourceManager> g_resources;
std::shared_ptr<IRenderer>       g_renderer;

Engine::Engine()
{
    LOG_INFO("Creating Engine");

    Time::SetStart();

    m_inputSettings = std::make_shared<InputSettings>();
    g_window        = std::make_shared<SDLWindow>(m_inputSettings);

    g_renderer = std::make_shared<Renderer>(static_pointer_cast<SDLWindow>(g_window));
    g_renderer->Setup();

    g_ui = std::make_shared<UI>();

    g_resources = std::make_shared<ResourceManager>();

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
    g_world.reset();
    g_ui.reset();
    g_window.reset();
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
        std::this_thread::yield();

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

    // Sleep before polling for input to reduce input lag.
    // In a FIFO (vsync) scenario, we are going to be blocked
    // by driver anyway either at image acquire or present,
    // which will effectively age our inputs by as much.
    // Pre-emptively sleep by previous frame block duration,
    // minus some headroom.
    // TODO: this could use some smooth heuristic
    // TODO: VK_EXT_present_timing ?
    const auto&  stats    = g_renderer->GetRenderStats();
    const auto   slop     = stats.AcquireBlockTime + stats.PresentBlockTime;
    const double headroom = 0.002; // 2ms
    const auto   sleep    = MIN(Time::AntiLag + slop - headroom, Time::FrameInterval - headroom);
    const auto   start    = Time::Now();
    while (Time::Now() < start + sleep)
    {
        std::this_thread::yield();
    }
    Time::AntiLag = Time::Now() - start;

    UpdateInput();

    const auto deltaTime = static_cast<float>(Time::DeltaFrame);

    m_app->Update(deltaTime);

    auto camera = g_renderer->GetCamera();
    camera->HandleInput(m_frameInput);

    if (m_frameInput.wantsResize)
    {
        int width;
        int height;
        g_window->GetFramebufferSize(&width, &height);
        camera->UpdateViewport(width, height);
        g_renderer->Resize();
    }

    g_world->Update(deltaTime);

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

        const auto deltaTime = static_cast<float>(Time::DeltaTick);
        m_app->FixedUpdate(deltaTime);
        g_world->FixedUpdate(deltaTime);

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

        g_renderer->PostProcess();

        g_renderer->BeginUI();
        g_ui->Render();
        g_renderer->EndUI();

        g_renderer->Submit();

        g_renderer->Present();

        Time::StopRender();

        m_mainFrameSemaphore.release();
    }

    LOG_INFO("Exit RenderThread");
}
} // namespace yar
