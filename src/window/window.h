#pragma once

#include <memory>

#include <SDL3/SDL_video.h>
#include <vulkan/vulkan_core.h>

#include "../public/input.h"
#include "../public/iwindow.h"

namespace yar
{
class SDLWindow : public IWindow
{
  public:
    SDLWindow(std::shared_ptr<InputSettings> inputSettings);
    ~SDLWindow();

    void SetTitle(const char* title) override;

    void SetMouseGrab(bool grab) override;
    bool IsMouseGrabbed() override;

    void GetFramebufferSize(int* width, int* height) override;

    bool IsMinimized() override;

    unsigned int GetRefreshRate() override;

    void AggregateInput(WindowInput& input);

    bool CreateVulkanSurface(VkInstance instance, VkSurfaceKHR* surface);

    const char* const* GetVulkanExtensions(unsigned int* pCount);

    SDL_Window* GetSDLWindow() const
    {
        return m_window;
    }

  private:
    SDL_Window*                    m_window;
    std::shared_ptr<InputSettings> m_inputSettings;
};
} // namespace yar
