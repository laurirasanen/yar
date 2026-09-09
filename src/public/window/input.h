#pragma once

#include <glm/vec2.hpp>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_scancode.h>

namespace yar
{

enum Key : unsigned int
{
    KEY_NONE,

    KEY_MOVE_FORWARD,
    KEY_MOVE_BACK,
    KEY_MOVE_RIGHT,
    KEY_MOVE_LEFT,
    KEY_MOVE_UP,
    KEY_MOVE_DOWN,

    KEY_INTERACT,

    KEY_MOUSE_GRAB,

    KEY_WINDOW_DEBUG,
    KEY_WINDOW_DEMO,

    KEY_EXPOSURE_UP,
    KEY_EXPOSURE_DOWN,
    KEY_CONTRAST_UP,
    KEY_CONTRAST_DOWN,

    KEY_IBL_UP,
    KEY_IBL_DOWN,

    KEY_SPEED_UP,
    KEY_SPEED_DOWN,

    KEY_MAX,
};

static constexpr unsigned long long KeyToFlag(Key k)
{
    return 1 << static_cast<unsigned int>(k);
}

class InputSettings
{
  public:
    InputSettings()
    {
        for (unsigned int i = 0; i < SDL_SCANCODE_COUNT; i++)
        {
            m_sdlKeyMap[i] = Key::KEY_NONE;
        }

        for (unsigned int i = 0; i < SDL_GAMEPAD_BUTTON_COUNT; i++)
        {
            m_sdlButtonMap[i] = Key::KEY_NONE;
        }

        ApplyDefaults();
    }

    void ApplyDefaults()
    {
        m_sdlKeyMap[static_cast<unsigned int>(SDL_SCANCODE_W)]     = Key::KEY_MOVE_FORWARD;
        m_sdlKeyMap[static_cast<unsigned int>(SDL_SCANCODE_A)]     = Key::KEY_MOVE_LEFT;
        m_sdlKeyMap[static_cast<unsigned int>(SDL_SCANCODE_S)]     = Key::KEY_MOVE_BACK;
        m_sdlKeyMap[static_cast<unsigned int>(SDL_SCANCODE_D)]     = Key::KEY_MOVE_RIGHT;
        m_sdlKeyMap[static_cast<unsigned int>(SDL_SCANCODE_SPACE)] = Key::KEY_MOVE_UP;
        m_sdlKeyMap[static_cast<unsigned int>(SDL_SCANCODE_LCTRL)] = Key::KEY_MOVE_DOWN;

        m_sdlKeyMap[static_cast<unsigned int>(SDL_SCANCODE_E)] = Key::KEY_INTERACT;

        m_sdlKeyMap[static_cast<unsigned int>(SDL_SCANCODE_F1)] = Key::KEY_MOUSE_GRAB;
        m_sdlKeyMap[static_cast<unsigned int>(SDL_SCANCODE_F2)] = Key::KEY_WINDOW_DEBUG;
        m_sdlKeyMap[static_cast<unsigned int>(SDL_SCANCODE_F3)] = Key::KEY_WINDOW_DEMO;

        m_sdlKeyMap[static_cast<unsigned int>(SDL_SCANCODE_1)] = Key::KEY_EXPOSURE_UP;
        m_sdlKeyMap[static_cast<unsigned int>(SDL_SCANCODE_2)] = Key::KEY_EXPOSURE_DOWN;
        m_sdlKeyMap[static_cast<unsigned int>(SDL_SCANCODE_3)] = Key::KEY_CONTRAST_UP;
        m_sdlKeyMap[static_cast<unsigned int>(SDL_SCANCODE_4)] = Key::KEY_CONTRAST_DOWN;

        m_sdlKeyMap[static_cast<unsigned int>(SDL_SCANCODE_5)] = Key::KEY_IBL_UP;
        m_sdlKeyMap[static_cast<unsigned int>(SDL_SCANCODE_6)] = Key::KEY_IBL_DOWN;

        m_sdlButtonMap[static_cast<unsigned int>(SDL_GAMEPAD_BUTTON_LEFT_SHOULDER)] =
            Key::KEY_SPEED_DOWN;
        m_sdlButtonMap[static_cast<unsigned int>(SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER)] =
            Key::KEY_SPEED_UP;

        m_sdlButtonMap[static_cast<unsigned int>(SDL_GAMEPAD_BUTTON_SOUTH)] = Key::KEY_INTERACT;
    }

    Key GetKeyFromSDL(unsigned int scan)
    {
        return m_sdlKeyMap[scan];
    }

    Key GetButtonFromSDL(unsigned int scan)
    {
        return m_sdlButtonMap[scan];
    }

  private:
    Key m_sdlKeyMap[SDL_SCANCODE_COUNT];
    Key m_sdlButtonMap[SDL_GAMEPAD_BUTTON_COUNT];
};

struct WindowInput
{
    bool wantsQuit;
    bool wantsResize;

    unsigned long long prevFlags;
    unsigned long long keyFlags;

    glm::vec2 mouse;
    glm::vec2 scroll;

    glm::vec2 joyLeft;
    glm::vec2 joyRight;

    float trigLeft;
    float trigRight;

    WindowInput()
    {
        prevFlags = 0;
        keyFlags  = 0;
        Clear(true);
    }

    void Clear(bool clearKeys = false)
    {
        wantsQuit   = false;
        wantsResize = false;

        prevFlags = keyFlags;

        // Generally don't want to do this,
        // since window only tracks key up / down events.
        if (clearKeys)
        {
            keyFlags = 0;

            joyLeft.x  = 0;
            joyLeft.y  = 0;
            joyRight.x = 0;
            joyRight.y = 0;

            trigLeft  = 0;
            trigRight = 0;
        }

        mouse.x  = 0;
        mouse.y  = 0;
        scroll.x = 0;
        scroll.y = 0;
    }

    void Aggregate(const WindowInput& other)
    {
        wantsQuit |= other.wantsQuit;
        wantsResize |= other.wantsResize;

        prevFlags |= other.prevFlags;
        keyFlags |= other.keyFlags;

        mouse += other.mouse;
        scroll += other.scroll;

        joyLeft  = other.joyLeft;
        joyRight = other.joyRight;

        trigLeft  = other.trigLeft;
        trigRight = other.trigRight;
    }

    bool IsDown(Key key) const
    {
        return (keyFlags & KeyToFlag(key)) == KeyToFlag(key);
    }

    bool WasPressed(Key key) const
    {
        const auto pressedPrev = (prevFlags & KeyToFlag(key)) == KeyToFlag(key);
        const auto pressedNow  = (keyFlags & KeyToFlag(key)) == KeyToFlag(key);
        return pressedNow && !pressedPrev;
    }

    void SetKeyDown(Key key)
    {
        keyFlags |= KeyToFlag(key);
    }

    void SetKeyUp(Key key)
    {
        keyFlags &= ~KeyToFlag(key);
    }
};
}; // namespace yar
