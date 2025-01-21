#include <zf4_window.h>

#include <zf4_game.h>

static enum key_code ConvertGLFWKeyCode(const int key_code) {
    switch (key_code) {
        case GLFW_KEY_SPACE: return key_code__space;

        case GLFW_KEY_0: return key_code__0;
        case GLFW_KEY_1: return key_code__1;
        case GLFW_KEY_2: return key_code__2;
        case GLFW_KEY_3: return key_code__3;
        case GLFW_KEY_4: return key_code__4;
        case GLFW_KEY_5: return key_code__5;
        case GLFW_KEY_6: return key_code__6;
        case GLFW_KEY_7: return key_code__7;
        case GLFW_KEY_8: return key_code__8;
        case GLFW_KEY_9: return key_code__9;

        case GLFW_KEY_A: return key_code__a;
        case GLFW_KEY_B: return key_code__b;
        case GLFW_KEY_C: return key_code__c;
        case GLFW_KEY_D: return key_code__d;
        case GLFW_KEY_E: return key_code__e;
        case GLFW_KEY_F: return key_code__f;
        case GLFW_KEY_G: return key_code__g;
        case GLFW_KEY_H: return key_code__h;
        case GLFW_KEY_I: return key_code__i;
        case GLFW_KEY_J: return key_code__j;
        case GLFW_KEY_K: return key_code__k;
        case GLFW_KEY_L: return key_code__l;
        case GLFW_KEY_M: return key_code__m;
        case GLFW_KEY_N: return key_code__n;
        case GLFW_KEY_O: return key_code__o;
        case GLFW_KEY_P: return key_code__p;
        case GLFW_KEY_Q: return key_code__q;
        case GLFW_KEY_R: return key_code__r;
        case GLFW_KEY_S: return key_code__s;
        case GLFW_KEY_T: return key_code__t;
        case GLFW_KEY_U: return key_code__u;
        case GLFW_KEY_V: return key_code__v;
        case GLFW_KEY_W: return key_code__w;
        case GLFW_KEY_X: return key_code__x;
        case GLFW_KEY_Y: return key_code__y;
        case GLFW_KEY_Z: return key_code__z;

        case GLFW_KEY_ESCAPE: return key_code__escape;
        case GLFW_KEY_ENTER: return key_code__enter;
        case GLFW_KEY_TAB: return key_code__tab;

        case GLFW_KEY_RIGHT: return key_code__right;
        case GLFW_KEY_LEFT: return key_code__left;
        case GLFW_KEY_DOWN: return key_code__down;
        case GLFW_KEY_UP: return key_code__up;

        case GLFW_KEY_F1: return key_code__f1;
        case GLFW_KEY_F2: return key_code__f2;
        case GLFW_KEY_F3: return key_code__f3;
        case GLFW_KEY_F4: return key_code__f4;
        case GLFW_KEY_F5: return key_code__f5;
        case GLFW_KEY_F6: return key_code__f6;
        case GLFW_KEY_F7: return key_code__f7;
        case GLFW_KEY_F8: return key_code__f8;
        case GLFW_KEY_F9: return key_code__f9;
        case GLFW_KEY_F10: return key_code__f10;
        case GLFW_KEY_F11: return key_code__f11;
        case GLFW_KEY_F12: return key_code__f12;

        case GLFW_KEY_LEFT_SHIFT: return key_code__left_shift;
        case GLFW_KEY_LEFT_CONTROL: return key_code__left_control;
        case GLFW_KEY_LEFT_ALT: return key_code__left_alt;

        default: return key_code__null;
    }
}

static enum mouse_button_code ConvertGLFWMouseButtonCode(const int button_code) {
    switch (button_code) {
        case GLFW_MOUSE_BUTTON_LEFT: return mouse_button_code__left;
        case GLFW_MOUSE_BUTTON_RIGHT: return mouse_button_code__right;
        case GLFW_MOUSE_BUTTON_MIDDLE: return mouse_button_code__middle;

        default: return mouse_button_code__null;
    }
}

static void GLFWWindowSizeCallback(GLFWwindow* const glfwWindow, const int width, const int height) {
    s_window* const window = glfwGetWindowUserPointer(glfwWindow);
    window->size_cache.x = width;
    window->size_cache.y = height;
}

static void GLFWKeyCallback(GLFWwindow* const glfwWindow, const int key, const int scancode, const int action, const int mods) {
    s_window* const window = glfwGetWindowUserPointer(glfwWindow);

    const enum key_code key_code = ConvertGLFWKeyCode(key);

    if (key_code) {
        const ta_keys_down_bits key_bit = (ta_keys_down_bits)1 << key_code;

        if (action == GLFW_PRESS) {
            window->input_state.keys_down |= key_bit;
        } else if (action == GLFW_RELEASE) {
            window->input_state.keys_down &= ~key_bit;
        }
    }
}

static void GLFWMouseButtonCallback(GLFWwindow* const glfwWindow, const int button, const int action, const int mods) {
    s_window* const window = glfwGetWindowUserPointer(glfwWindow);

    const enum mouse_button_code button_code = ConvertGLFWMouseButtonCode(button);

    if (button_code) {
        const ta_mouse_buttons_down_bits button_bit = (ta_mouse_buttons_down_bits)1 << button_code;

        if (action == GLFW_PRESS) {
            window->input_state.mouse_buttons_down |= button_bit;
        } else if (action == GLFW_RELEASE) {
            window->input_state.mouse_buttons_down &= ~button_bit;
        }
    }
}

static void GLFWCursorPosCallback(GLFWwindow* const glfwWindow, const double x, const double y) {
    s_window* const window = glfwGetWindowUserPointer(glfwWindow);
    window->input_state.mouse_pos.x = x;
    window->input_state.mouse_pos.y = y;
}

bool InitWindow(s_window* const window, const s_vec_2d_i size, const char* const title, const enum window_flags flags) {
    assert(window);
    assert(IsClear(window, sizeof(*window)));
    assert(size.x > 0 && size.y > 0);
    assert(title);
    assert(flags >= 0 && flags < (1 << window_flags_cnt));

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, ZF4_GL_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, ZF4_GL_VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, flags & window_flags__resizable);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    window->glfw_window = glfwCreateWindow(size.x, size.y, title, NULL, NULL);

    if (!window->glfw_window) {
        LogError("Failed to create GLFW window!");
        return false;
    }

    glfwMakeContextCurrent(window->glfw_window);

    glfwSwapInterval(1); // Enables VSync.

    glfwSetWindowUserPointer(window->glfw_window, window);
    glfwSetWindowSizeCallback(window->glfw_window, GLFWWindowSizeCallback);
    glfwSetKeyCallback(window->glfw_window, GLFWKeyCallback);
    glfwSetMouseButtonCallback(window->glfw_window, GLFWMouseButtonCallback);
    glfwSetCursorPosCallback(window->glfw_window, GLFWCursorPosCallback);

    glfwSetInputMode(window->glfw_window, GLFW_CURSOR, (flags & window_flags__hide_cursor) ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);

    // Get the initial window size (as it otherwise won't be set until the callback).
    glfwGetWindowSize(window->glfw_window, &window->size_cache.x, &window->size_cache.y);

    return true;
}

void CleanWindow(s_window* const window) {
    assert(window);

    if (window->glfw_window) {
        glfwDestroyWindow(window->glfw_window);
    }

    Clear(window, sizeof(*window));
}
