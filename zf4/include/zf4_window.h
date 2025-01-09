#pragma once

#include <cassert>
#include <GLFW/glfw3.h>
#include <zf4c.h>

namespace zf4 {
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

    class Window {
    public:
        static bool init(const int width, const int height, const char* const title, const bool resizable, const bool hideCursor);
        static void clean();

        static Vec2DI get_size() {
            assert(s_glfwWindow);

            Vec2DI size;
            glfwGetWindowSize(s_glfwWindow, &size.x, &size.y);

            return size;
        }

        static void show() {
            assert(s_glfwWindow);
            glfwShowWindow(s_glfwWindow);
        }

        static bool should_close() {
            assert(s_glfwWindow);
            return glfwWindowShouldClose(s_glfwWindow);
        }

        static void swap_buffers() {
            assert(s_glfwWindow);
            glfwSwapBuffers(s_glfwWindow);
        }

        static void save_input_state() {
            assert(s_glfwWindow);
            s_inputStateSaved = s_inputState;
        }

        static bool is_key_down(const KeyCode keyCode) {
            assert(s_glfwWindow);
            const KeysDownBitset keyBit = static_cast<KeysDownBitset>(1) << keyCode;
            return s_inputState.keysDown & keyBit;
        }

        static bool is_key_pressed(const KeyCode keyCode) {
            assert(s_glfwWindow);
            const KeysDownBitset keyBit = static_cast<KeysDownBitset>(1) << keyCode;
            return (s_inputState.keysDown & keyBit) && !(s_inputStateSaved.keysDown & keyBit);
        }

        static bool is_key_released(const KeyCode keyCode) {
            assert(s_glfwWindow);
            const KeysDownBitset keyBit = static_cast<KeysDownBitset>(1) << keyCode;
            return !(s_inputState.keysDown & keyBit) && (s_inputStateSaved.keysDown & keyBit);
        }

        static bool is_mouse_button_down(const MouseButtonCode buttonCode) {
            assert(s_glfwWindow);
            const MouseButtonsDownBitset buttonBit = static_cast<MouseButtonsDownBitset>(1) << buttonCode;
            return s_inputState.mouseButtonsDown & buttonBit;
        }

        static bool is_mouse_button_pressed(const MouseButtonCode buttonCode) {
            assert(s_glfwWindow);
            const MouseButtonsDownBitset buttonBit = static_cast<MouseButtonsDownBitset>(1) << buttonCode;
            return (s_inputState.mouseButtonsDown & buttonBit) && !(s_inputStateSaved.mouseButtonsDown & buttonBit);
        }

        static bool is_mouse_button_released(const MouseButtonCode buttonCode) {
            assert(s_glfwWindow);
            const MouseButtonsDownBitset buttonBit = static_cast<MouseButtonsDownBitset>(1) << buttonCode;
            return !(s_inputState.mouseButtonsDown & buttonBit) && (s_inputStateSaved.mouseButtonsDown & buttonBit);
        }

        static Vec2D get_mouse_pos() {
            assert(s_glfwWindow);
            return s_inputState.mousePos;
        }

    private:
        static inline GLFWwindow* s_glfwWindow;

        static inline InputState s_inputState;
        static inline InputState s_inputStateSaved;

        static void glfw_key_callback(GLFWwindow* const window, const int key, const int scancode, const int action, const int mods);
        static void glfw_mouse_button_callback(GLFWwindow* const window, const int button, const int action, const int mods);
        static void glfw_cursor_pos_callback(GLFWwindow* const window, const double x, const double y);
    };
}
