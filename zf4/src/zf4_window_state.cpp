#include <zf4_window.h>

#include <cassert>
#include <glad/glad.h>
#include <zf4c.h>

namespace zf4 {
    typedef unsigned long long KeysDownBitset;
    typedef unsigned char MouseButtonsDownBitset;

    typedef struct {
        KeysDownBitset keysDown;
        MouseButtonsDownBitset mouseButtonsDown;
        Vec2D mousePos;
    } InputState;

    static GLFWwindow* i_glfwWindow;
    static Pt2D i_windowSize;
    static InputState i_inputState;
    static InputState i_inputStateSaved;

    static void glfw_window_size_callback(GLFWwindow* const window, const int width, const int height) {
        i_windowSize.x = width;
        i_windowSize.y = height;
        glViewport(0, 0, width, height); // TODO: Move elsewhere?
    }

    static void glfw_key_callback(GLFWwindow* const window, const int key, const int scancode, const int action, const int mods) {
        const KeyCode keyCode = conv_glfw_key_code(key);

        if (keyCode != UNDEFINED_KEY_CODE) {
            const KeysDownBitset keyBit = (KeysDownBitset)1 << keyCode;

            if (action == GLFW_PRESS) {
                i_inputState.keysDown |= keyBit;
            } else if (action == GLFW_RELEASE) {
                i_inputState.keysDown &= ~keyBit;
            }
        }
    }

    static void glfw_mouse_button_callback(GLFWwindow* const window, const int button, const int action, const int mods) {
        const MouseButtonCode buttonCode = conv_glfw_mouse_button_code(button);

        if (buttonCode != UNDEFINED_MOUSE_BUTTON_CODE) {
            const MouseButtonsDownBitset buttonBit = (MouseButtonsDownBitset)1 << buttonCode;

            if (action == GLFW_PRESS) {
                i_inputState.mouseButtonsDown |= buttonBit;
            } else if (action == GLFW_RELEASE) {
                i_inputState.mouseButtonsDown &= ~buttonBit;
            }
        }
    }

    static void glfw_cursor_pos_callback(GLFWwindow* const window, const double x, const double y) {
        i_inputState.mousePos.x = (float)x;
        i_inputState.mousePos.y = (float)y;
    }

    bool init_window(const int width, const int height, const char* const title, const bool resizable, const bool hideCursor) {
        assert(!i_glfwWindow);
        assert(width > 0 && height > 0);
        assert(title && title[0]);

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RESIZABLE, resizable);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        i_glfwWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);

        if (!i_glfwWindow) {
            log_error("Failed to create GLFW window!");
            return false;
        }

        glfwMakeContextCurrent(i_glfwWindow);

        glfwSetWindowSizeCallback(i_glfwWindow, glfw_window_size_callback);
        glfwSetKeyCallback(i_glfwWindow, glfw_key_callback);
        glfwSetMouseButtonCallback(i_glfwWindow, glfw_mouse_button_callback);
        glfwSetCursorPosCallback(i_glfwWindow, glfw_cursor_pos_callback);

        glfwSetInputMode(i_glfwWindow, GLFW_CURSOR, hideCursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);

        glfwGetWindowSize(i_glfwWindow, &i_windowSize.x, &i_windowSize.y);

        return true;
    }

    void clean_window() {
        if (i_glfwWindow) {
            glfwDestroyWindow(i_glfwWindow);
            i_glfwWindow = nullptr;
        }
    }

    Pt2D get_window_size() {
        return i_windowSize;
    }

    void show_window() {
        glfwShowWindow(i_glfwWindow);
    }

    bool window_should_close() {
        return glfwWindowShouldClose(i_glfwWindow);
    }

    void swap_window_buffers() {
        glfwSwapBuffers(i_glfwWindow);
    }

    void save_input_state() {
        i_inputStateSaved = i_inputState;
    }

    bool is_key_down(const KeyCode keyCode) {
        const KeysDownBitset keyBit = (KeysDownBitset)1 << keyCode;
        return i_inputState.keysDown & keyBit;
    }

    bool is_key_pressed(const KeyCode keyCode) {
        const KeysDownBitset keyBit = (KeysDownBitset)1 << keyCode;
        return (i_inputState.keysDown & keyBit) && !(i_inputStateSaved.keysDown & keyBit);
    }

    bool is_key_released(const KeyCode keyCode) {
        const KeysDownBitset keyBit = (KeysDownBitset)1 << keyCode;
        return !(i_inputState.keysDown & keyBit) && (i_inputStateSaved.keysDown & keyBit);
    }

    bool is_mouse_button_down(const MouseButtonCode buttonCode) {
        const MouseButtonsDownBitset buttonBit = (MouseButtonsDownBitset)1 << buttonCode;
        return i_inputState.mouseButtonsDown & buttonBit;
    }

    bool is_mouse_button_pressed(const MouseButtonCode buttonCode) {
        const MouseButtonsDownBitset buttonBit = (MouseButtonsDownBitset)1 << buttonCode;
        return (i_inputState.mouseButtonsDown & buttonBit) && !(i_inputStateSaved.mouseButtonsDown & buttonBit);
    }

    bool is_mouse_button_released(const MouseButtonCode buttonCode) {
        const MouseButtonsDownBitset buttonBit = (MouseButtonsDownBitset)1 << buttonCode;
        return !(i_inputState.mouseButtonsDown & buttonBit) && (i_inputStateSaved.mouseButtonsDown & buttonBit);
    }

    Vec2D get_mouse_pos() {
        return i_inputState.mousePos;
    }
}
