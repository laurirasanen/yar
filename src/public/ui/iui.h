#pragma once

#include <memory>
#include <string>

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

class IUI
{
  public:
    IUI()          = default;
    virtual ~IUI() = default;

    IUI(const IUI&)            = delete;
    IUI(IUI&&)                 = delete;
    IUI& operator=(const IUI&) = delete;
    IUI& operator=(IUI&&)      = delete;

    virtual void ShowLoadingScreen() = 0;
    virtual void HideLoadingScreen() = 0;

    virtual void ToggleWindow(UIWindow window) = 0;

    virtual void SetLoadingText(std::string text) = 0;

    virtual void Render() = 0;
};

extern std::shared_ptr<IUI> g_ui;
}; // namespace yar
