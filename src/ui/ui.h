#pragma once

#include <memory>

#include <glm/vec2.hpp>

#include "..//renderer/common.h"
#include "../public/iui.h"

namespace yar
{
class UI : public IUI
{
  public:
    UI();
    ~UI();

    UI(const UI&)            = delete;
    UI(UI&&)                 = delete;
    UI& operator=(const UI&) = delete;
    UI& operator=(UI&&)      = delete;

    void ShowLoadingScreen() override
    {
        if (!m_state.showWindow[static_cast<unsigned int>(UIWindow::LOADING)])
        {
            ToggleWindow(UIWindow::LOADING);
        }
    }

    void HideLoadingScreen() override
    {
        if (m_state.showWindow[static_cast<unsigned int>(UIWindow::LOADING)])
        {
            ToggleWindow(UIWindow::LOADING);
        }
    }

    void ToggleWindow(UIWindow window) override
    {
        auto index                = static_cast<unsigned int>(window);
        m_state.showWindow[index] = !m_state.showWindow[index];

        if (window == UIWindow::LOADING)
        {
            if (m_state.showWindow[static_cast<unsigned int>(UIWindow::LOADING)])
            {
                m_loadingStartTime = Time::Now();
            }
            else
            {
                LOG_INFO("Finished loading in {:.2f} seconds", Time::Now() - m_loadingStartTime);
            }
        }
    }

    void SetLoadingText(std::string text) override
    {
        m_loadingText = text;
    }

    void Render() override;

  private:
    void DebugWindow();
    void LoadingWindow();
    void DemoWindow();

    VulkanImGuiCreationInfo m_info;

    UIState m_state;

    std::string m_loadingText;
    double      m_loadingStartTime;
};
}; // namespace yar
