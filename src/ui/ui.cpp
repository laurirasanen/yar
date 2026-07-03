#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>
#include <imgui_internal.h>
#include <SDL3/SDL_events.h>

#include "../platform/memory.h"
#include "../public/iphysics.h"
#include "../public/log.h"
#include "../renderer/renderer.h"
#include "ui.h"

namespace yar
{

UI::UI() : m_state({})
{
    LOG_INFO("Creating UI");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigDebugIsDebuggerPresent = true;

    const auto window = static_pointer_cast<SDLWindow>(g_window);
    ImGui_ImplSDL3_InitForVulkan(window->GetSDLWindow());
    const auto renderer = static_pointer_cast<Renderer>(g_renderer);
    renderer->GetImGuiInfo(m_info);
    ImGui_ImplVulkan_Init(&m_info.imInit);

    m_state.showWindow[static_cast<unsigned int>(UIWindow::DEBUG)] = true;
}

UI::~UI()
{
    LOG_INFO("Destroying UI");
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

void UI::Render()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    DebugWindow();
    LoadingWindow();
    DemoWindow();

    ImGui::Render();

    auto data     = ImGui::GetDrawData();
    auto renderer = std::static_pointer_cast<Renderer>(g_renderer);
    ImGui_ImplVulkan_RenderDrawData(data, renderer->GetVkCommandBuffer());
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
                | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs
        ))
    {
        const auto renderStats = g_renderer->GetRenderStats();
        const auto cullStats   = g_renderer->GetCullStats();

        ImGui::Text("FPS: %.2f (%.2fms)", 1.0 / Time::DeltaFrame, Time::DeltaFrame * 1000.0);
        ImGui::Text("  Render: %.2fms", Time::DeltaRender * 1000.0);
        ImGui::Text("  Cull: %.2fms", cullStats.CullTime * 1000.0);
        ImGui::Text("  Sort: %.2fms", renderStats.SortTime * 1000.0);

        ImGui::Text("TPS: %.0f (%.2fms)", 1.0 / Time::DeltaTick, Time::DeltaTick * 1000.0);

        const auto phyStats = g_physics->GetStats();
        ImGui::Text("Physics:");
        ImGui::Text("  Memory: %s", Memory::Pretty(phyStats.MemoryBytes).c_str());
        ImGui::Text("  Bodies: %u", phyStats.Bodies);
        ImGui::Text("  Shapes: %u", phyStats.Shapes);
        ImGui::Text(
            "  Contacts: %u (a: %u, r: %u)",
            phyStats.Contacts,
            phyStats.ContactsAwake,
            phyStats.ContactsRecycled
        );
        ImGui::Text("  Joints: %u", phyStats.Joints);
        ImGui::Text("  Islands: %u", phyStats.Islands);
        ImGui::Text("  Tasks: %u", phyStats.Tasks);
        ImGui::Text("  Time: %.2fms", phyStats.UpdateTime * 1000.0);

        const auto vkStats = GetVulkanAllocatorTotalStatistics();
        ImGui::Text("Memory:");
        ImGui::Text("  Resident: %s", Memory::GetPrettyUsage().c_str());
        ImGui::Text(
            "  Vulkan: %s",
            Memory::Pretty(vkStats.total.statistics.allocationBytes).c_str()
        );

        ImGui::Text("Visible:");
        ImGui::Text("  Meshes: %zu", renderStats.MeshCount);
        ImGui::Text("  Indices: %zu", renderStats.IndexCount);
        ImGui::Text("  Vertices: %zu", renderStats.VertexCount);

        ImGui::Text("Culled:");
        ImGui::Text("  Meshes: %zu", cullStats.MeshCount);
        ImGui::Text("  Indices: %zu", cullStats.IndexCount);
        ImGui::Text("  Vertices: %zu", cullStats.VertexCount);

        ImGui::Text("Camera:");
        const auto camera = g_renderer->GetCamera();
        if (camera == nullptr)
        {
            ImGui::Text("  None");
        }
        else
        {
            const auto pos = camera->transform.GetPosition();
            ImGui::Text(
                "  pos: [%.2f, %.2f, %.2f]",
                static_cast<double>(pos.x),
                static_cast<double>(pos.y),
                static_cast<double>(pos.z)
            );
            ImGui::Text(
                "  ang: [%.2f, %.2f, %.2f]",
                static_cast<double>(camera->Pitch),
                static_cast<double>(camera->Yaw),
                static_cast<double>(camera->Roll)
            );
        }

        ImGui::End();
    }
}

void UI::LoadingWindow()
{
    if (!m_state.showWindow[static_cast<unsigned int>(UIWindow::LOADING)])
    {
        return;
    }

    ImGui::SetNextWindowBgAlpha(0.0f);
    const auto center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::Begin(
            "Loading info",
            &m_state.showWindow[static_cast<unsigned int>(UIWindow::LOADING)],
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground
                | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs
        ))
    {
        ImGui::SetWindowFontScale(3.0f);
        ImGui::Text("Loading...");
        ImGui::SetWindowFontScale(1.0f);
        ImGui::Text("elapsed: %.2fs", Time::Now() - m_loadingStartTime);
        ImGui::Text("%s", m_loadingText.c_str());
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
}; // namespace yar
