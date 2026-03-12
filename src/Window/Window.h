#pragma once

#include <memory>
#include <SDL_syswm.h>
#include <vulkan/vulkan_core.h>

#include "Input.h"

namespace drive
{
class Window
{
  public:
    Window(std::shared_ptr<InputSettings> inputSettings);
    ~Window();

    void SetMouseGrab(bool grab);
    bool IsMouseGrabbed();

    void AggregateInput(WindowInput& input);

    void GetFramebufferSize(int* width, int* height);

    bool CreateVulkanSurface(VkInstance instance, VkSurfaceKHR* surface);

    bool GetVulkanExtensions(unsigned int* pCount, const char** pNames);

    bool IsMinimized();

    unsigned int GetRefreshRate();

    SDL_Window* GetSDLWindow() const
    {
        return m_window;
    }

  private:
    SDL_Window*                    m_window;
    std::shared_ptr<InputSettings> m_inputSettings;
};
} // namespace drive
