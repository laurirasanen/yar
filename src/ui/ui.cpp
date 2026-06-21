#include <cstdint>
#include <stdexcept>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>
#include <imgui_internal.h>
#include <SDL3/SDL_events.h>

#include "../log.h"
#include "../platform/memory.h"
#include "ui.h"

namespace yar
{

UI::UI(
    std::shared_ptr<Window>   window,
    std::shared_ptr<Renderer> renderer,
    std::shared_ptr<Camera>   camera
) :
    m_window(window),
    m_renderer(renderer),
    m_camera(camera),
    m_state({})
{
    LOG_INFO("Creating UI");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigDebugIsDebuggerPresent = true;

    ImGui_ImplSDL3_InitForVulkan(m_window->GetSDLWindow());
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
    auto renderer = std::static_pointer_cast<Renderer>(m_renderer);
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
        const auto fps = std::format(
            "FPS: {:.0f} ({:.2f} ms)",
            1.0 / Time::DeltaFrame,
            Time::DeltaFrame * 1000.0
        );
        ImGui::Text("%s", fps.c_str());

        const auto ren = std::format("  Render: {:.2f} ms", Time::DeltaRender * 1000.0);
        ImGui::Text("%s", ren.c_str());

        const auto tps =
            std::format("TPS: {:.0f} ({:.2f} ms)", 1.0 / Time::DeltaTick, Time::DeltaTick * 1000.0);
        ImGui::Text("%s", tps.c_str());

        const auto mem     = std::format("Resident: {}", Memory::GetPrettyUsage());
        const auto vkStats = GetVulkanAllocatorTotalStatistics();
        const auto vkMem =
            std::format("Vulkan: {}", Memory::Pretty(vkStats.total.statistics.allocationBytes));
        ImGui::Text("Memory:");
        ImGui::Text("  %s", mem.c_str());
        ImGui::Text("  %s", vkMem.c_str());

        const auto renderStats = m_renderer->GetRenderStats();
        ImGui::Text("Visible:");
        ImGui::Text("  Meshes: %zu", renderStats.MeshCount);
        ImGui::Text("  Indices: %zu", renderStats.IndexCount);
        ImGui::Text("  Vertices: %zu", renderStats.VertexCount);

        const auto cullStats = m_renderer->GetCullStats();
        ImGui::Text("Culled:");
        ImGui::Text("  Meshes: %zu", cullStats.MeshCount);
        ImGui::Text("  Indices: %zu", cullStats.IndexCount);
        ImGui::Text("  Vertices: %zu", cullStats.VertexCount);

        const auto pos     = m_camera->transform.GetPosition();
        const auto ang     = m_camera->transform.GetEulerRotation();
        const auto posText = std::format("pos: [{:.2f}, {:.2f}, {:.2f}]", pos.x, pos.y, pos.z);
        const auto angText = std::format("ang: [{:.2f}, {:.2f}, {:.2f}]", ang.x, ang.y, ang.z);
        ImGui::Text("Camera:");
        ImGui::Text("  %s", posText.c_str());
        ImGui::Text("  %s", angText.c_str());

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
        ImGui::Text("model: %s", m_loadingModel.c_str());
        ImGui::Text("mesh: %s", m_loadingMesh.c_str());
        ImGui::Text("material: %s", m_loadingMaterial.c_str());
        ImGui::Text("texture: %s", m_loadingTexture.c_str());
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
