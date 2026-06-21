#pragma once

#include <memory>

#include <glm/vec2.hpp>

#include "../renderer/renderer.h"
#include "../window/window.h"

namespace yar
{

enum class UIWindow
{
    DEBUG,
    LOADING,
    DEMO,
    MAX
};

struct UIState
{
    bool showWindow[static_cast<unsigned int>(UIWindow::MAX)];
};

class UI
{
  public:
    UI()                     = delete;
    UI(const UI&)            = delete;
    UI(UI&&)                 = delete;
    UI& operator=(const UI&) = delete;
    UI& operator=(UI&&)      = delete;

    UI(std::shared_ptr<Window>   window,
       std::shared_ptr<Renderer> renderer,
       std::shared_ptr<Camera>   camera);
    ~UI();

    void ToggleWindow(UIWindow window)
    {
        auto index                = static_cast<unsigned int>(window);
        m_state.showWindow[index] = !m_state.showWindow[index];
    }

    void SetLoadingModel(std::string model)
    {
        m_loadingModel = model;
    }

    void SetLoadingMesh(std::string mesh)
    {
        m_loadingMesh = mesh;
    }

    void SetLoadingMaterial(std::string material)
    {
        m_loadingMaterial = material;
    }

    void SetLoadingTexture(std::string texture)
    {
        m_loadingTexture = texture;
    }

    void Render();

  private:
    void DebugWindow();
    void LoadingWindow();
    void DemoWindow();

    std::shared_ptr<Window>   m_window;
    std::shared_ptr<Renderer> m_renderer;
    std::shared_ptr<Camera>   m_camera;

    VulkanImGuiCreationInfo m_info;

    UIState m_state;

    std::string m_loadingModel;
    std::string m_loadingMesh;
    std::string m_loadingMaterial;
    std::string m_loadingTexture;
};
}; // namespace yar
