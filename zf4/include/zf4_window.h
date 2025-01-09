#pragma once

#include <cassert>
#include <GLFW/glfw3.h>
#include <zf4c.h>

namespace zf4 {
    enum KeyCode {
        UNDEFINED_KEY_CODE = -1,

        KEY_SPACE,

        KEY_0,
        KEY_1,
        KEY_2,
        KEY_3,
        KEY_4,
        KEY_5,
        KEY_6,
        KEY_7,
        KEY_8,
        KEY_9,

        KEY_A,
        KEY_B,
        KEY_C,
        KEY_D,
        KEY_E,
        KEY_F,
        KEY_G,
        KEY_H,
        KEY_I,
        KEY_J,
        KEY_K,
        KEY_L,
        KEY_M,
        KEY_N,
        KEY_O,
        KEY_P,
        KEY_Q,
        KEY_R,
        KEY_S,
        KEY_T,
        KEY_U,
        KEY_V,
        KEY_W,
        KEY_X,
        KEY_Y,
        KEY_Z,

        KEY_ESCAPE,
        KEY_ENTER,
        KEY_TAB,

        KEY_RIGHT,
        KEY_LEFT,
        KEY_DOWN,
        KEY_UP,

        KEY_F1,
        KEY_F2,
        KEY_F3,
        KEY_F4,
        KEY_F5,
        KEY_F6,
        KEY_F7,
        KEY_F8,
        KEY_F9,
        KEY_F10,
        KEY_F11,
        KEY_F12,

        KEY_LEFT_SHIFT,
        KEY_LEFT_CONTROL,
        KEY_LEFT_ALT,

        NUM_KEY_CODES
    };

    enum MouseButtonCode {
        UNDEFINED_MOUSE_BUTTON_CODE = -1,

        MOUSE_BUTTON_LEFT,
        MOUSE_BUTTON_RIGHT,
        MOUSE_BUTTON_MID,

        NUM_MOUSE_BUTTON_CODES
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
            return s_windowSize;
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
        static inline Vec2DI s_windowSize;
        static inline InputState s_inputState;
        static inline InputState s_inputStateSaved;

        static void glfw_window_size_callback(GLFWwindow* const window, const int width, const int height);
        static void glfw_key_callback(GLFWwindow* const window, const int key, const int scancode, const int action, const int mods);
        static void glfw_mouse_button_callback(GLFWwindow* const window, const int button, const int action, const int mods);
        static void glfw_cursor_pos_callback(GLFWwindow* const window, const double x, const double y);
    };
}
