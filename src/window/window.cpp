#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>

#include "../log.h"
#include "window.h"

namespace yar
{
Window::Window(std::shared_ptr<InputSettings> inputSettings) : m_inputSettings(inputSettings)
{
    LOG_INFO(
        "Creating window, SDL version: {}.{}.{}",
        SDL_MAJOR_VERSION,
        SDL_MINOR_VERSION,
        SDL_MICRO_VERSION
    );

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        LOG_ERROR("SDL could not initialize. SDL_Error: {}", SDL_GetError());
        throw("Failed to create Engine");
    }

    m_window = SDL_CreateWindow("yar", 1920, 1080, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

    if (m_window == nullptr)
    {
        LOG_ERROR("Window could not be created. SDL_Error: {}", SDL_GetError());
        throw("Failed to create window");
    }

    LOG_DEBUG("Window created");
}

Window::~Window()
{
    LOG_INFO("Destroying window");
    if (m_window != nullptr)
    {
        SDL_DestroyWindow(m_window);
    }
    SDL_Quit();
}

void Window::SetMouseGrab(bool grab)
{
    SDL_SetWindowRelativeMouseMode(m_window, grab);
}

bool Window::IsMouseGrabbed()
{
    return SDL_GetWindowRelativeMouseMode(m_window);
}

void Window::AggregateInput(WindowInput& input)
{
    ImGuiIO& io             = ImGui::GetIO();
    bool     handleKeyboard = true;
    bool     handleMouse    = true;

    for (SDL_Event event; SDL_PollEvent(&event) != 0;)
    {
        if (event.type == SDL_EVENT_QUIT)
        {
            input.wantsQuit = true;
            break;
        }

        ImGui_ImplSDL3_ProcessEvent(&event);
        handleKeyboard = !io.WantCaptureKeyboard;
        handleMouse    = !io.WantCaptureMouse;

        if (!handleMouse)
        {
            SetMouseGrab(false);
        }
        if (!IsMouseGrabbed())
        {
            handleKeyboard = false;
        }

        if (event.type >= SDL_EVENT_WINDOW_FIRST && event.type <= SDL_EVENT_WINDOW_LAST)
        {
            switch (event.window.type)
            {
                case SDL_EVENT_WINDOW_RESIZED:
                    input.wantsResize = true;
                    break;

                default:
                    break;
            }
        }

        if (handleKeyboard)
        {
            if (event.type == SDL_EVENT_KEY_DOWN)
            {
                auto key = m_inputSettings->GetKeyFromSDL(event.key.scancode);
                input.KeyDown(key);
            }

            if (event.type == SDL_EVENT_KEY_UP)
            {
                auto key = m_inputSettings->GetKeyFromSDL(event.key.scancode);
                input.KeyUp(key);
            }
        }

        if (handleMouse)
        {
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
            {
                switch (event.button.button)
                {
                    case 1:
                        SetMouseGrab(true);
                        break;

                    default:
                        break;
                }
            }

            if (IsMouseGrabbed())
            {
                if (event.type == SDL_EVENT_MOUSE_MOTION)
                {
                    input.mouse.x += event.motion.xrel;
                    input.mouse.y += event.motion.yrel;
                }
                if (event.type == SDL_EVENT_MOUSE_WHEEL)
                {
                    input.scroll.x += event.motion.xrel;
                    input.scroll.y += event.motion.yrel;
                }
            }
        }
    }
}

void Window::GetFramebufferSize(int* width, int* height)
{
    SDL_GetWindowSizeInPixels(m_window, width, height);
}

bool Window::CreateVulkanSurface(VkInstance instance, VkSurfaceKHR* surface)
{
    // TODO alloc cb
    return SDL_Vulkan_CreateSurface(m_window, instance, nullptr, surface);
}

const char* const* Window::GetVulkanExtensions(unsigned int* pCount)
{
    return SDL_Vulkan_GetInstanceExtensions(pCount);
}

bool Window::IsMinimized()
{
    return SDL_GetWindowFlags(m_window) & SDL_WINDOW_MINIMIZED;
}

unsigned int Window::GetRefreshRate()
{
    auto display = SDL_GetDisplayForWindow(m_window);
    if (display != 0)
    {
        if (const auto* mode = SDL_GetDesktopDisplayMode(display))
        {
            return static_cast<unsigned int>(mode->refresh_rate);
        }
    }

    LOG_ERROR("Failed to get display mode, {}", SDL_GetError());
    return 60;
}
} // namespace yar
