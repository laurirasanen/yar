#pragma once

#include <memory>

#include <SDL3/SDL_video.h>
#include <vulkan/vulkan_core.h>

#include "input.h"

namespace yar
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

    const char* const* GetVulkanExtensions(unsigned int* pCount);

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
} // namespace yar
