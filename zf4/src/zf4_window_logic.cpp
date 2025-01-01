#include <zf4_window.h>

ZF4KeyCode zf4_conv_glfw_key_code(const int keyCode) {
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

ZF4MouseButtonCode zf4_conv_glfw_mouse_button_code(const int buttonCode) {
    switch (buttonCode) {
        case GLFW_MOUSE_BUTTON_LEFT: return ZF4_MOUSE_BUTTON_LEFT;
        case GLFW_MOUSE_BUTTON_RIGHT: return ZF4_MOUSE_BUTTON_RIGHT;
        case GLFW_MOUSE_BUTTON_MIDDLE: return ZF4_MOUSE_BUTTON_MID;

        default: return ZF4_UNDEFINED_MOUSE_BUTTON_CODE;
    }
}
