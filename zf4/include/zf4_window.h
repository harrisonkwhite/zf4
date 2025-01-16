#pragma once

#include <cassert>
#include <GLFW/glfw3.h>
#include <zf4c.h>

namespace zf4 {
    enum WindowFlags {
        WindowFlags_HideCursor = 1 << 0,
        WindowFlags_Resizable = 1 << 1
    };

    enum KeyCode {
        UndefinedKeyCode = -1,

        KeyCode_Space,

        KeyCode_0,
        KeyCode_1,
        KeyCode_2,
        KeyCode_3,
        KeyCode_4,
        KeyCode_5,
        KeyCode_6,
        KeyCode_7,
        KeyCode_8,
        KeyCode_9,

        KeyCode_A,
        KeyCode_B,
        KeyCode_C,
        KeyCode_D,
        KeyCode_E,
        KeyCode_F,
        KeyCode_G,
        KeyCode_H,
        KeyCode_I,
        KeyCode_J,
        KeyCode_K,
        KeyCode_L,
        KeyCode_M,
        KeyCode_N,
        KeyCode_O,
        KeyCode_P,
        KeyCode_Q,
        KeyCode_R,
        KeyCode_S,
        KeyCode_T,
        KeyCode_U,
        KeyCode_V,
        KeyCode_W,
        KeyCode_X,
        KeyCode_Y,
        KeyCode_Z,

        KeyCode_Escape,
        KeyCode_Enter,
        KeyCode_Tab,

        KeyCode_Right,
        KeyCode_Left,
        KeyCode_Down,
        KeyCode_Up,

        KeyCode_F1,
        KeyCode_F2,
        KeyCode_F3,
        KeyCode_F4,
        KeyCode_F5,
        KeyCode_F6,
        KeyCode_F7,
        KeyCode_F8,
        KeyCode_F9,
        KeyCode_F10,
        KeyCode_F11,
        KeyCode_F12,

        KeyCode_LeftShift,
        KeyCode_LeftControl,
        KeyCode_LeftAlt,

        KeyCodeCnt
    };

    enum MouseButtonCode {
        UndefinedMouseButtonCode = -1,

        MouseButtonCode_Left,
        MouseButtonCode_Right,
        MouseButtonCode_Middle,

        MouseButtonCodeCnt
    };

    using KeysDownBitset = unsigned long long;
    using MouseButtonsDownBitset = unsigned char;

    struct InputState {
        KeysDownBitset keysDown;
        MouseButtonsDownBitset mouseButtonsDown;
        Vec2D mousePos;
    };

    struct Window {
        GLFWwindow* glfwWindow;
        Vec2DI size; // glfwGetWindowSize() is relatively expensive for some reason, so we cache this.

        InputState inputState;
        InputState inputStateSaved; // Used as a reference point for press and release checks, among other things.
    };

    bool init_window(Window* const window, const zf4::Vec2DI size, const char* const title, const WindowFlags flags);

    inline bool is_key_down(const KeyCode keyCode, const Window* const window) {
        const KeysDownBitset keyBit = static_cast<KeysDownBitset>(1) << keyCode;
        return window->inputState.keysDown & keyBit;
    }

    inline bool is_key_pressed(const KeyCode keyCode, const Window* const window) {
        const KeysDownBitset keyBit = static_cast<KeysDownBitset>(1) << keyCode;
        return (window->inputState.keysDown & keyBit) && !(window->inputStateSaved.keysDown & keyBit);
    }

    inline bool is_key_released(const KeyCode keyCode, const Window* const window) {
        const KeysDownBitset keyBit = static_cast<KeysDownBitset>(1) << keyCode;
        return !(window->inputState.keysDown & keyBit) && (window->inputStateSaved.keysDown & keyBit);
    }

    inline bool is_mouse_button_down(const MouseButtonCode buttonCode, const Window* const window) {
        const MouseButtonsDownBitset buttonBit = static_cast<MouseButtonsDownBitset>(1) << buttonCode;
        return window->inputState.mouseButtonsDown & buttonBit;
    }

    inline bool is_mouse_button_pressed(const MouseButtonCode buttonCode, const Window* const window) {
        const MouseButtonsDownBitset buttonBit = static_cast<MouseButtonsDownBitset>(1) << buttonCode;
        return (window->inputState.mouseButtonsDown & buttonBit) && !(window->inputStateSaved.mouseButtonsDown & buttonBit);
    }

    inline bool is_mouse_button_released(const MouseButtonCode buttonCode, const Window* const window) {
        const MouseButtonsDownBitset buttonBit = static_cast<MouseButtonsDownBitset>(1) << buttonCode;
        return !(window->inputState.mouseButtonsDown & buttonBit) && (window->inputStateSaved.mouseButtonsDown & buttonBit);
    }
}
