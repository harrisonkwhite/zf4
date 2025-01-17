#include <zf4_window.h>

#include <zf4_game.h>

namespace zf4 {
    static KeyCode conv_glfw_key_code(const int keyCode) {
        switch (keyCode) {
            case GLFW_KEY_SPACE: return KeyCode_Space;

            case GLFW_KEY_0: return KeyCode_0;
            case GLFW_KEY_1: return KeyCode_1;
            case GLFW_KEY_2: return KeyCode_2;
            case GLFW_KEY_3: return KeyCode_3;
            case GLFW_KEY_4: return KeyCode_4;
            case GLFW_KEY_5: return KeyCode_5;
            case GLFW_KEY_6: return KeyCode_6;
            case GLFW_KEY_7: return KeyCode_7;
            case GLFW_KEY_8: return KeyCode_8;
            case GLFW_KEY_9: return KeyCode_9;

            case GLFW_KEY_A: return KeyCode_A;
            case GLFW_KEY_B: return KeyCode_B;
            case GLFW_KEY_C: return KeyCode_C;
            case GLFW_KEY_D: return KeyCode_D;
            case GLFW_KEY_E: return KeyCode_E;
            case GLFW_KEY_F: return KeyCode_F;
            case GLFW_KEY_G: return KeyCode_G;
            case GLFW_KEY_H: return KeyCode_H;
            case GLFW_KEY_I: return KeyCode_I;
            case GLFW_KEY_J: return KeyCode_J;
            case GLFW_KEY_K: return KeyCode_K;
            case GLFW_KEY_L: return KeyCode_L;
            case GLFW_KEY_M: return KeyCode_M;
            case GLFW_KEY_N: return KeyCode_N;
            case GLFW_KEY_O: return KeyCode_O;
            case GLFW_KEY_P: return KeyCode_P;
            case GLFW_KEY_Q: return KeyCode_Q;
            case GLFW_KEY_R: return KeyCode_R;
            case GLFW_KEY_S: return KeyCode_S;
            case GLFW_KEY_T: return KeyCode_T;
            case GLFW_KEY_U: return KeyCode_U;
            case GLFW_KEY_V: return KeyCode_V;
            case GLFW_KEY_W: return KeyCode_W;
            case GLFW_KEY_X: return KeyCode_X;
            case GLFW_KEY_Y: return KeyCode_Y;
            case GLFW_KEY_Z: return KeyCode_Z;

            case GLFW_KEY_ESCAPE: return KeyCode_Escape;
            case GLFW_KEY_ENTER: return KeyCode_Enter;
            case GLFW_KEY_TAB: return KeyCode_Tab;

            case GLFW_KEY_RIGHT: return KeyCode_Right;
            case GLFW_KEY_LEFT: return KeyCode_Left;
            case GLFW_KEY_DOWN: return KeyCode_Down;
            case GLFW_KEY_UP: return KeyCode_Up;

            case GLFW_KEY_F1: return KeyCode_F1;
            case GLFW_KEY_F2: return KeyCode_F2;
            case GLFW_KEY_F3: return KeyCode_F3;
            case GLFW_KEY_F4: return KeyCode_F4;
            case GLFW_KEY_F5: return KeyCode_F5;
            case GLFW_KEY_F6: return KeyCode_F6;
            case GLFW_KEY_F7: return KeyCode_F7;
            case GLFW_KEY_F8: return KeyCode_F8;
            case GLFW_KEY_F9: return KeyCode_F9;
            case GLFW_KEY_F10: return KeyCode_F10;
            case GLFW_KEY_F11: return KeyCode_F11;
            case GLFW_KEY_F12: return KeyCode_F12;

            case GLFW_KEY_LEFT_SHIFT: return KeyCode_LeftShift;
            case GLFW_KEY_LEFT_CONTROL: return KeyCode_LeftControl;
            case GLFW_KEY_LEFT_ALT: return KeyCode_LeftAlt;

            default: return UndefinedKeyCode;
        }
    }

    static MouseButtonCode conv_glfw_mouse_button_code(const int buttonCode) {
        switch (buttonCode) {
            case GLFW_MOUSE_BUTTON_LEFT: return MouseButtonCode_Left;
            case GLFW_MOUSE_BUTTON_RIGHT: return MouseButtonCode_Right;
            case GLFW_MOUSE_BUTTON_MIDDLE: return MouseButtonCode_Middle;

            default: return UndefinedMouseButtonCode;
        }
    }

    static void glfw_window_size_callback(GLFWwindow* const glfwWindow, const int width, const int height) {
        const auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
        window->size.x = width;
        window->size.y = height;
    }

    static void glfw_key_callback(GLFWwindow* const glfwWindow, const int key, const int scancode, const int action, const int mods) {
        const auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));

        const KeyCode keyCode = conv_glfw_key_code(key);

        if (keyCode != UndefinedKeyCode) {
            const KeysDownBitset keyBit = static_cast<KeysDownBitset>(1) << keyCode;

            if (action == GLFW_PRESS) {
                window->inputState.keysDown |= keyBit;
            } else if (action == GLFW_RELEASE) {
                window->inputState.keysDown &= ~keyBit;
            }
        }
    }

    static void glfw_mouse_button_callback(GLFWwindow* const glfwWindow, const int button, const int action, const int mods) {
        const auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));

        const MouseButtonCode buttonCode = conv_glfw_mouse_button_code(button);

        if (buttonCode != UndefinedMouseButtonCode) {
            const MouseButtonsDownBitset buttonBit = static_cast<MouseButtonsDownBitset>(1) << buttonCode;

            if (action == GLFW_PRESS) {
                window->inputState.mouseButtonsDown |= buttonBit;
            } else if (action == GLFW_RELEASE) {
                window->inputState.mouseButtonsDown &= ~buttonBit;
            }
        }
    }

    static void glfw_cursor_pos_callback(GLFWwindow* const glfwWindow, const double x, const double y) {
        const auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
        window->inputState.mousePos.x = x;
        window->inputState.mousePos.y = y;
    }

    bool init_window(Window& window, const zf4::Vec2DI size, const char* const title, const WindowFlags flags) {
        assert(is_zero(&window));
        assert(size.x > 0 && size.y > 0);

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gk_glVersionMajor);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gk_glVersionMinor);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RESIZABLE, flags & WindowFlags_Resizable);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        window.glfwWindow = glfwCreateWindow(size.x, size.y, title, nullptr, nullptr);

        if (!window.glfwWindow) {
            log_error("Failed to create GLFW window!");
            return false;
        }

        glfwMakeContextCurrent(window.glfwWindow);

        glfwSwapInterval(1); // Enables VSync.

        glfwSetWindowUserPointer(window.glfwWindow, &window);
        glfwSetWindowSizeCallback(window.glfwWindow, glfw_window_size_callback);
        glfwSetKeyCallback(window.glfwWindow, glfw_key_callback);
        glfwSetMouseButtonCallback(window.glfwWindow, glfw_mouse_button_callback);
        glfwSetCursorPosCallback(window.glfwWindow, glfw_cursor_pos_callback);

        glfwSetInputMode(window.glfwWindow, GLFW_CURSOR, (flags & WindowFlags_HideCursor) ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);

        glfwGetWindowSize(window.glfwWindow, &window.size.x, &window.size.y);

        return true;
    }
}
