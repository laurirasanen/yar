#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>

#include "../public/log.h"
#include "window.h"

#include <cmath>

namespace yar
{
SDLWindow::SDLWindow(std::shared_ptr<InputSettings> inputSettings) : m_inputSettings(inputSettings)
{
    LOG_INFO(
        "Creating window, SDL version: {}.{}.{}",
        SDL_MAJOR_VERSION,
        SDL_MINOR_VERSION,
        SDL_MICRO_VERSION
    );

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
        LOG_ERROR("SDL could not initialize. SDL_Error: {}", SDL_GetError());
        throw("Failed to create Engine");
    }

    m_window = SDL_CreateWindow(
        "yar",
        1920,
        1080,
#if NDEBUG
        SDL_WINDOW_VULKAN | SDL_WINDOW_FULLSCREEN
#else
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
#endif
    );

    if (m_window == nullptr)
    {
        LOG_ERROR("Window could not be created. SDL_Error: {}", SDL_GetError());
        throw("Failed to create window");
    }

    LOG_DEBUG("Window created");
}

SDLWindow::~SDLWindow()
{
    LOG_INFO("Destroying window");
    if (m_window != nullptr)
    {
        SDL_DestroyWindow(m_window);
    }
    SDL_Quit();
}

void SDLWindow::SetTitle(const char* title)
{
    SDL_SetWindowTitle(m_window, title);
}

void SDLWindow::SetMouseGrab(bool grab)
{
    SDL_SetWindowRelativeMouseMode(m_window, grab);
}

bool SDLWindow::IsMouseGrabbed()
{
    return SDL_GetWindowRelativeMouseMode(m_window);
}

void SDLWindow::ConnectGamepads()
{
    if (!SDL_HasGamepad())
    {
        LOG_DEBUG("No gamepads");
        return;
    }

    int             count = 0;
    SDL_JoystickID* joys  = SDL_GetGamepads(&count);
    if (joys == nullptr)
    {
        LOG_ERROR("Failed to get gamepads, {}", SDL_GetError());
        return;
    }

    LOG_DEBUG("Found {} gamepads", count);

    for (int i = 0; i < count; i++)
    {
        auto* pad = SDL_OpenGamepad(joys[i]);
        if (pad == nullptr)
        {
            LOG_ERROR("Failed to open gamepad {}: {}", i, SDL_GetError());
        }
        else
        {
            LOG_DEBUG("Opened gamepad '{}'", SDL_GetGamepadName(pad));
        }
    }

    SDL_free(joys);
}

void SDLWindow::AggregateInput(WindowInput& input)
{
    ImGuiIO& io             = ImGui::GetIO();
    bool     handleKeyboard = true;
    bool     handleMouse    = true;
    bool     handleGamepad  = true;

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
                input.SetKeyDown(key);
            }

            if (event.type == SDL_EVENT_KEY_UP)
            {
                auto key = m_inputSettings->GetKeyFromSDL(event.key.scancode);
                input.SetKeyUp(key);
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
                    const float dir = event.wheel.direction == SDL_MOUSEWHEEL_NORMAL ? 1.0f : -1.0f;
                    input.scroll.x += dir * event.wheel.x;
                    input.scroll.y += dir * event.wheel.y;
                }
            }
        }

        if (handleGamepad)
        {
            const int deadZone = static_cast<int>(0.15f * SDL_JOYSTICK_AXIS_MAX);
            if (event.type == SDL_EVENT_GAMEPAD_AXIS_MOTION)
            {
                float* axis = nullptr;

                if (event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTX)
                {
                    axis = &input.joyLeft.x;
                }
                else if (event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTY)
                {
                    axis = &input.joyLeft.y;
                }
                else if (event.gaxis.axis == SDL_GAMEPAD_AXIS_RIGHTX)
                {
                    axis = &input.joyRight.x;
                }
                else if (event.gaxis.axis == SDL_GAMEPAD_AXIS_RIGHTY)
                {
                    axis = &input.joyRight.y;
                }
                else if (event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER)
                {
                    axis = &input.trigLeft;
                }
                else if (event.gaxis.axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER)
                {
                    axis = &input.trigRight;
                }

                if (axis != nullptr)
                {
                    if (abs(event.gaxis.value) > deadZone)
                    {
                        *axis = static_cast<float>(event.gaxis.value) / SDL_JOYSTICK_AXIS_MAX;
                    }
                    else
                    {
                        *axis = 0;
                    }
                }
            }

            if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN)
            {
                auto key = m_inputSettings->GetButtonFromSDL(event.gbutton.button);
                input.SetKeyDown(key);
            }

            if (event.type == SDL_EVENT_GAMEPAD_BUTTON_UP)
            {
                auto key = m_inputSettings->GetButtonFromSDL(event.gbutton.button);
                input.SetKeyUp(key);
            }
        }
        else
        {
            input.joyLeft.x  = 0;
            input.joyLeft.y  = 0;
            input.joyRight.x = 0;
            input.joyRight.y = 0;

            input.trigRight = 0;
            input.trigRight = 0;
        }
    }
}

void SDLWindow::GetFramebufferSize(int* width, int* height)
{
    SDL_GetWindowSizeInPixels(m_window, width, height);
}

bool SDLWindow::CreateVulkanSurface(VkInstance instance, VkSurfaceKHR* surface)
{
    return SDL_Vulkan_CreateSurface(m_window, instance, nullptr, surface);
}

const char* const* SDLWindow::GetVulkanExtensions(unsigned int* pCount)
{
    return SDL_Vulkan_GetInstanceExtensions(pCount);
}

bool SDLWindow::IsMinimized()
{
    return SDL_GetWindowFlags(m_window) & SDL_WINDOW_MINIMIZED;
}

unsigned int SDLWindow::GetRefreshRate()
{
    auto display = SDL_GetDisplayForWindow(m_window);
    if (display != 0)
    {
        if (const auto* mode = SDL_GetDesktopDisplayMode(display))
        {
            if (mode->refresh_rate_numerator <= 0)
            {
                LOG_ERROR("Unspecified display refresh rate");
                return 60;
            }

            float rate = static_cast<float>(mode->refresh_rate_numerator)
                         / static_cast<float>(mode->refresh_rate_denominator);
            rate       = std::ceilf(rate);
            return static_cast<unsigned int>(rate);
        }
    }

    LOG_ERROR("Failed to get display mode, {}", SDL_GetError());
    return 60;
}
} // namespace yar
