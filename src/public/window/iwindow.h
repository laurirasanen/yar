#pragma once

#include <memory>

namespace yar
{
class IWindow
{
  public:
    IWindow()          = default;
    virtual ~IWindow() = default;

    IWindow(const IWindow&)            = delete;
    IWindow(IWindow&&)                 = delete;
    IWindow& operator=(const IWindow&) = delete;
    IWindow& operator=(IWindow&&)      = delete;

    virtual void SetTitle(const char* title) = 0;

    virtual void SetMouseGrab(bool grab) = 0;
    virtual bool IsMouseGrabbed()        = 0;

    virtual void GetFramebufferSize(int* width, int* height) = 0;

    virtual bool IsMinimized() = 0;

    virtual unsigned int GetRefreshRate() = 0;
};

extern std::shared_ptr<IWindow> g_window;
}; // namespace yar
