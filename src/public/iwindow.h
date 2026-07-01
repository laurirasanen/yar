#pragma once

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
};
}; // namespace yar
