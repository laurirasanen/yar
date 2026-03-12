#include <stdexcept>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>
#include <imgui_internal.h>
#include <SDL_events.h>

#include "../Log.h"
#include "../Memory.h"
#include "UI.h"

namespace drive
{

UI::UI(std::shared_ptr<Window> window, std::shared_ptr<Renderer> renderer) :
    m_window(window),
    m_renderer(renderer),
    m_state({})
{
    LOG_INFO("Creating UI");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigDebugIsDebuggerPresent = true;

    switch (m_renderer->Type())
    {
        case RendererType::EMPTY:
        {
            ImGui_ImplSDL2_InitForOther(m_window->GetSDLWindow());
            break;
        }

        case RendererType::VULKAN:
        {
            ImGui_ImplSDL2_InitForVulkan(m_window->GetSDLWindow());
            auto vulkanRenderer = std::static_pointer_cast<VulkanRenderer>(m_renderer);
            vulkanRenderer->GetImGuiInfo(m_info);
            ImGui_ImplVulkan_Init(&m_info.imGuiInfo);
            break;
        }

        default:
            throw std::logic_error("implement me");
    }

    m_state.showWindow[static_cast<unsigned int>(UIWindow::DEBUG)] = true;
}

UI::~UI()
{
    LOG_INFO("Destroying UI");

    switch (m_renderer->Type())
    {
        case RendererType::EMPTY:
        {
            break;
        }

        case RendererType::VULKAN:
        {
            ImGui_ImplVulkan_Shutdown();
            break;
        }

        default:
        {
            LOG_ERROR("implement me");
            break;
        }
    }

    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void UI::Render()
{
    // Start new frame
    switch (m_renderer->Type())
    {
        case RendererType::EMPTY:
        {
            return;
        }

        case RendererType::VULKAN:
        {
            ImGui_ImplVulkan_NewFrame();
            break;
        }

        default:
            throw std::logic_error("implement me");
    }
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    DebugWindow();
    DemoWindow();

    // Prep data for renderer implementation
    ImGui::Render();

    // Pass the data to renderer
    switch (m_renderer->Type())
    {
        case RendererType::EMPTY:
        {
            break;
        }

        case RendererType::VULKAN:
        {
            auto data           = ImGui::GetDrawData();
            auto vulkanRenderer = std::static_pointer_cast<VulkanRenderer>(m_renderer);
            ImGui_ImplVulkan_RenderDrawData(data, vulkanRenderer->GetVkCommandBuffer());
            break;
        }

        default:
            throw std::logic_error("implement me");
    }
}

void UI::DebugWindow()
{
    if (!m_state.showWindow[static_cast<unsigned int>(UIWindow::DEBUG)])
    {
        return;
    }

    ImGui::SetNextWindowBgAlpha(0.5f);
    if (ImGui::Begin(
            "Debug info",
            &m_state.showWindow[static_cast<unsigned int>(UIWindow::DEBUG)],
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav
                | ImGuiWindowFlags_NoDecoration
        ))
    {
        auto fps = std::format(
            "FPS: {:.0f} ({:.2f} ms)",
            1.0 / Time::DeltaFrame,
            Time::DeltaFrame * 1000.0
        );
        ImGui::Text("%s", fps.c_str());

        auto ren = std::format("  Render: {:.2f} ms", Time::DeltaRender * 1000.0);
        ImGui::Text("%s", ren.c_str());

        auto tps =
            std::format("TPS: {:.0f} ({:.2f} ms)", 1.0 / Time::DeltaTick, Time::DeltaTick * 1000.0);
        ImGui::Text("%s", tps.c_str());

        auto mem = std::format("MEM: {:d} MB", Memory::GetUsage() / 1024);
        ImGui::Text("%s", mem.c_str());

        ImGui::End();
    }
}

void UI::DemoWindow()
{
    if (!m_state.showWindow[static_cast<unsigned int>(UIWindow::DEMO)])
    {
        return;
    }

    ImGui::ShowDemoWindow(&m_state.showWindow[static_cast<unsigned int>(UIWindow::DEMO)]);
}
}; // namespace drive
