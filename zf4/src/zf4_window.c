#include <zf4_window.h>

#include <assert.h>
#include <glad/glad.h>
#include <zf4c.h>

typedef unsigned long long KeysDownBitset;
typedef unsigned char MouseButtonsDownBitset;

typedef struct {
    KeysDownBitset keysDown;
    MouseButtonsDownBitset mouseButtonsDown;
    ZF4Vec2D mousePos;
} InputState;

static GLFWwindow* i_glfwWindow;
static ZF4Pt2D i_windowSize;
static InputState i_inputState;
static InputState i_inputStateSaved;

static ZF4KeyCode glfw_to_zf4_key_code(const int keyCode) {
    switch (keyCode) {
        case GLFW_KEY_SPACE: return ZF4_KEY_SPACE;

        case GLFW_KEY_0: return ZF4_KEY_0;
        case GLFW_KEY_1: return ZF4_KEY_1;
        case GLFW_KEY_2: return ZF4_KEY_2;
        case GLFW_KEY_3: return ZF4_KEY_3;
        case GLFW_KEY_4: return ZF4_KEY_4;
        case GLFW_KEY_5: return ZF4_KEY_5;
        case GLFW_KEY_6: return ZF4_KEY_6;
        case GLFW_KEY_7: return ZF4_KEY_7;
        case GLFW_KEY_8: return ZF4_KEY_8;
        case GLFW_KEY_9: return ZF4_KEY_9;

        case GLFW_KEY_A: return ZF4_KEY_A;
        case GLFW_KEY_B: return ZF4_KEY_B;
        case GLFW_KEY_C: return ZF4_KEY_C;
        case GLFW_KEY_D: return ZF4_KEY_D;
        case GLFW_KEY_E: return ZF4_KEY_E;
        case GLFW_KEY_F: return ZF4_KEY_F;
        case GLFW_KEY_G: return ZF4_KEY_G;
        case GLFW_KEY_H: return ZF4_KEY_H;
        case GLFW_KEY_I: return ZF4_KEY_I;
        case GLFW_KEY_J: return ZF4_KEY_J;
        case GLFW_KEY_K: return ZF4_KEY_K;
        case GLFW_KEY_L: return ZF4_KEY_L;
        case GLFW_KEY_M: return ZF4_KEY_M;
        case GLFW_KEY_N: return ZF4_KEY_N;
        case GLFW_KEY_O: return ZF4_KEY_O;
        case GLFW_KEY_P: return ZF4_KEY_P;
        case GLFW_KEY_Q: return ZF4_KEY_Q;
        case GLFW_KEY_R: return ZF4_KEY_R;
        case GLFW_KEY_S: return ZF4_KEY_S;
        case GLFW_KEY_T: return ZF4_KEY_T;
        case GLFW_KEY_U: return ZF4_KEY_U;
        case GLFW_KEY_V: return ZF4_KEY_V;
        case GLFW_KEY_W: return ZF4_KEY_W;
        case GLFW_KEY_X: return ZF4_KEY_X;
        case GLFW_KEY_Y: return ZF4_KEY_Y;
        case GLFW_KEY_Z: return ZF4_KEY_Z;

        case GLFW_KEY_ESCAPE: return ZF4_KEY_ESCAPE;
        case GLFW_KEY_ENTER: return ZF4_KEY_ENTER;
        case GLFW_KEY_TAB: return ZF4_KEY_TAB;

        case GLFW_KEY_RIGHT: return ZF4_KEY_RIGHT;
        case GLFW_KEY_LEFT: return ZF4_KEY_LEFT;
        case GLFW_KEY_DOWN: return ZF4_KEY_DOWN;
        case GLFW_KEY_UP: return ZF4_KEY_UP;

        case GLFW_KEY_F1: return ZF4_KEY_F1;
        case GLFW_KEY_F2: return ZF4_KEY_F2;
        case GLFW_KEY_F3: return ZF4_KEY_F3;
        case GLFW_KEY_F4: return ZF4_KEY_F4;
        case GLFW_KEY_F5: return ZF4_KEY_F5;
        case GLFW_KEY_F6: return ZF4_KEY_F6;
        case GLFW_KEY_F7: return ZF4_KEY_F7;
        case GLFW_KEY_F8: return ZF4_KEY_F8;
        case GLFW_KEY_F9: return ZF4_KEY_F9;
        case GLFW_KEY_F10: return ZF4_KEY_F10;
        case GLFW_KEY_F11: return ZF4_KEY_F11;
        case GLFW_KEY_F12: return ZF4_KEY_F12;

        case GLFW_KEY_LEFT_SHIFT: return ZF4_KEY_LEFT_SHIFT;
        case GLFW_KEY_LEFT_CONTROL: return ZF4_KEY_LEFT_CONTROL;
        case GLFW_KEY_LEFT_ALT: return ZF4_KEY_LEFT_ALT;

        default: return ZF4_UNDEFINED_KEY_CODE;
    }
}

static ZF4MouseButtonCode glfw_to_zf4_mouse_button_code(const int buttonCode) {
    switch (buttonCode) {
        case GLFW_MOUSE_BUTTON_LEFT: return ZF4_MOUSE_BUTTON_LEFT;
        case GLFW_MOUSE_BUTTON_RIGHT: return ZF4_MOUSE_BUTTON_RIGHT;
        case GLFW_MOUSE_BUTTON_MIDDLE: return ZF4_MOUSE_BUTTON_MID;

        default: return ZF4_UNDEFINED_MOUSE_BUTTON_CODE;
    }
}

static void glfw_window_size_callback(GLFWwindow* const window, const int width, const int height) {
    i_windowSize.x = width;
    i_windowSize.y = height;
    glViewport(0, 0, width, height); // TODO: Move elsewhere?
}

static void glfw_key_callback(GLFWwindow* const window, const int key, const int scancode, const int action, const int mods) {
    const ZF4KeyCode keyCode = glfw_to_zf4_key_code(key);

    if (keyCode != ZF4_UNDEFINED_KEY_CODE) {
        const KeysDownBitset keyBit = (KeysDownBitset)1 << keyCode;

        if (action == GLFW_PRESS) {
            i_inputState.keysDown |= keyBit;
        } else if (action == GLFW_RELEASE) {
            i_inputState.keysDown &= ~keyBit;
        }
    }
}

static void glfw_mouse_button_callback(GLFWwindow* const window, const int button, const int action, const int mods) {
    const ZF4MouseButtonCode buttonCode = glfw_to_zf4_mouse_button_code(button);

    if (buttonCode != ZF4_UNDEFINED_MOUSE_BUTTON_CODE) {
        const MouseButtonsDownBitset buttonBit = (MouseButtonsDownBitset)(1) << buttonCode;

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

bool zf4_init_window(const int width, const int height, const char* const title, const bool resizable, const bool hideCursor) {
    assert(!i_glfwWindow);
    assert(width > 0 && height > 0);
    assert(title && title[0]);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, resizable);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    i_glfwWindow = glfwCreateWindow(width, height, title, NULL, NULL);

    if (!i_glfwWindow) {
        zf4_log_error("Failed to create GLFW window!");
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

void zf4_clean_window() {
    if (i_glfwWindow) {
        glfwDestroyWindow(i_glfwWindow);
        i_glfwWindow = NULL;
    }
}

ZF4Pt2D zf4_get_window_size() {
    return i_windowSize;
}

void zf4_show_window() {
    glfwShowWindow(i_glfwWindow);
}

bool zf4_window_should_close() {
    return glfwWindowShouldClose(i_glfwWindow);
}

void zf4_swap_window_buffers() {
    glfwSwapBuffers(i_glfwWindow);
}

void zf4_save_input_state() {
    i_inputStateSaved = i_inputState;
}

bool zf4_is_key_down(const ZF4KeyCode keyCode) {
    const KeysDownBitset keyBit = (KeysDownBitset)1 << keyCode;
    return i_inputState.keysDown & keyBit;
}

bool zf4_is_key_pressed(const ZF4KeyCode keyCode) {
    const KeysDownBitset keyBit = (KeysDownBitset)1 << keyCode;
    return (i_inputState.keysDown & keyBit) && !(i_inputStateSaved.keysDown & keyBit);
}

bool zf4_is_key_released(const ZF4KeyCode keyCode) {
    const KeysDownBitset keyBit = (KeysDownBitset)1 << keyCode;
    return !(i_inputState.keysDown & keyBit) && (i_inputStateSaved.keysDown & keyBit);
}

bool zf4_is_mouse_button_down(const ZF4MouseButtonCode buttonCode) {
    const MouseButtonsDownBitset buttonBit = (MouseButtonsDownBitset)1 << buttonCode;
    return i_inputState.mouseButtonsDown & buttonBit;
}

bool zf4_is_mouse_button_pressed(const ZF4MouseButtonCode buttonCode) {
    const MouseButtonsDownBitset buttonBit = (MouseButtonsDownBitset)1 << buttonCode;
    return (i_inputState.mouseButtonsDown & buttonBit) && !(i_inputStateSaved.mouseButtonsDown & buttonBit);
}

bool zf4_is_mouse_button_released(const ZF4MouseButtonCode buttonCode) {
    const MouseButtonsDownBitset buttonBit = (MouseButtonsDownBitset)1 << buttonCode;
    return !(i_inputState.mouseButtonsDown & buttonBit) && (i_inputStateSaved.mouseButtonsDown & buttonBit);
}

ZF4Vec2D zf4_get_mouse_pos() {
    return i_inputState.mousePos;
}
