#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>
#include <imgui_internal.h>
#include <SDL3/SDL_events.h>

#include "../platform/memory.h"
#include "../public/iphysics.h"
#include "../public/log.h"
#include "../public/util.h"
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

    ImGui::SetNextWindowBgAlpha(0.75f);

    if (ImGui::Begin(
            "Debug info",
            &m_state.showWindow[static_cast<unsigned int>(UIWindow::DEBUG)],
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav
                | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs
        ))
    {
        const auto renderStats = g_renderer->GetRenderStats();

        ImGui::Text(
            "FPS: %.2f (%s)",
            1.0 / Time::DeltaFrame,
            Time::Pretty(Time::DeltaFrame).c_str()
        );
        ImGui::Text("  Render: %s", Time::Pretty(Time::DeltaRender).c_str());
        ImGui::Text("  Scene:");
        ImGui::Text("    Update: %s", Time::Pretty(renderStats.SceneUpdateTime).c_str());
        ImGui::Text("    Cull: %s", Time::Pretty(renderStats.SceneCullTime).c_str());
        ImGui::Text("    Batch: %s", Time::Pretty(renderStats.SceneBatchTime).c_str());
        ImGui::Text("    Sort: %s", Time::Pretty(renderStats.SceneSortTime).c_str());
        ImGui::Text("    Desc: %s", Time::Pretty(renderStats.SceneDescriptorTime).c_str());
        ImGui::Text("    Render: %s", Time::Pretty(renderStats.SceneRenderTime).c_str());
        ImGui::Text("    PP: %s", Time::Pretty(renderStats.PostProcessTime).c_str());

        ImGui::Text("TPS: %.0f (%s)", 1.0 / Time::DeltaTick, Time::Pretty(Time::DeltaTick).c_str());

        const auto phyStats = g_physics->GetStats();
        ImGui::Text("Physics:");
        ImGui::Text("  Memory: %s", Memory::Pretty(phyStats.MemoryBytes).c_str());
        ImGui::Text("  Bodies: %s", Numbers::Pretty(phyStats.Bodies).c_str());
        ImGui::Text("  Shapes: %s", Numbers::Pretty(phyStats.Shapes).c_str());
        ImGui::Text("  Contacts: %s", Numbers::Pretty(phyStats.Contacts).c_str());
        ImGui::Text("    Awake: %s", Numbers::Pretty(phyStats.ContactsAwake).c_str());
        ImGui::Text("    Recycled: %s", Numbers::Pretty(phyStats.ContactsRecycled).c_str());
        ImGui::Text("  Joints: %s", Numbers::Pretty(phyStats.Joints).c_str());
        ImGui::Text("  Islands: %s", Numbers::Pretty(phyStats.Islands).c_str());
        ImGui::Text("  Tasks: %s", Numbers::Pretty(phyStats.Tasks).c_str());
        ImGui::Text("  Time: %s", Time::Pretty(phyStats.UpdateTime).c_str());

        const auto vkStats = GetVulkanAllocatorTotalStatistics();
        ImGui::Text("Memory:");
        ImGui::Text("  Resident: %s", Memory::GetPrettyUsage().c_str());
        ImGui::Text(
            "  Vulkan: %s",
            Memory::Pretty(vkStats.total.statistics.allocationBytes).c_str()
        );

        ImGui::Text("Visible:");
        ImGui::Text("  Nodes: %s", Numbers::Pretty(renderStats.NodeCount).c_str());
        ImGui::Text("  Vertices: %s", Numbers::Pretty(renderStats.VertexCount).c_str());
        ImGui::Text("  Indices: %s", Numbers::Pretty(renderStats.IndexCount).c_str());
        ImGui::Text("  Textures: %s", Numbers::Pretty(renderStats.TextureCount).c_str());

        ImGui::Text("Culled:");
        ImGui::Text("  Nodes: %s", Numbers::Pretty(renderStats.CulledNodeCount).c_str());
        ImGui::Text("  Vertices: %s", Numbers::Pretty(renderStats.CulledVertexCount).c_str());
        ImGui::Text("  Indices: %s", Numbers::Pretty(renderStats.CulledIndexCount).c_str());

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
