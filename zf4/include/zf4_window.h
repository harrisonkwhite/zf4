#ifndef ZF4_WINDOW_H
#define ZF4_WINDOW_H

#include <stdbool.h>
#include <GLFW/glfw3.h>
#include <zf4c_math.h>

typedef enum {
    ZF4_UNDEFINED_KEY_CODE = -1,

    ZF4_KEY_SPACE,

    ZF4_KEY_0,
    ZF4_KEY_1,
    ZF4_KEY_2,
    ZF4_KEY_3,
    ZF4_KEY_4,
    ZF4_KEY_5,
    ZF4_KEY_6,
    ZF4_KEY_7,
    ZF4_KEY_8,
    ZF4_KEY_9,

    ZF4_KEY_A,
    ZF4_KEY_B,
    ZF4_KEY_C,
    ZF4_KEY_D,
    ZF4_KEY_E,
    ZF4_KEY_F,
    ZF4_KEY_G,
    ZF4_KEY_H,
    ZF4_KEY_I,
    ZF4_KEY_J,
    ZF4_KEY_K,
    ZF4_KEY_L,
    ZF4_KEY_M,
    ZF4_KEY_N,
    ZF4_KEY_O,
    ZF4_KEY_P,
    ZF4_KEY_Q,
    ZF4_KEY_R,
    ZF4_KEY_S,
    ZF4_KEY_T,
    ZF4_KEY_U,
    ZF4_KEY_V,
    ZF4_KEY_W,
    ZF4_KEY_X,
    ZF4_KEY_Y,
    ZF4_KEY_Z,

    ZF4_KEY_ESCAPE,
    ZF4_KEY_ENTER,
    ZF4_KEY_TAB,

    ZF4_KEY_RIGHT,
    ZF4_KEY_LEFT,
    ZF4_KEY_DOWN,
    ZF4_KEY_UP,

    ZF4_KEY_F1,
    ZF4_KEY_F2,
    ZF4_KEY_F3,
    ZF4_KEY_F4,
    ZF4_KEY_F5,
    ZF4_KEY_F6,
    ZF4_KEY_F7,
    ZF4_KEY_F8,
    ZF4_KEY_F9,
    ZF4_KEY_F10,
    ZF4_KEY_F11,
    ZF4_KEY_F12,

    ZF4_KEY_LEFT_SHIFT,
    ZF4_KEY_LEFT_CONTROL,
    ZF4_KEY_LEFT_ALT,

    ZF4_NUM_KEY_CODES
} ZF4KeyCode;

typedef enum {
    ZF4_UNDEFINED_MOUSE_BUTTON_CODE = -1,

    ZF4_MOUSE_BUTTON_LEFT,
    ZF4_MOUSE_BUTTON_RIGHT,
    ZF4_MOUSE_BUTTON_MID,

    ZF4_NUM_MOUSE_BUTTON_CODES
} ZF4MouseButtonCode;

bool zf4_init_window(const int width, const int height, const char* const title, const bool resizable, const bool hideCursor);
void zf4_clean_window();
ZF4Pt2D zf4_get_window_size();
void zf4_show_window();
bool zf4_window_should_close();
void zf4_swap_window_buffers();

void zf4_save_input_state();
bool zf4_is_key_down(const ZF4KeyCode keyCode);
bool zf4_is_key_pressed(const ZF4KeyCode keyCode);
bool zf4_is_key_released(const ZF4KeyCode keyCode);
bool zf4_is_mouse_button_down(const ZF4MouseButtonCode buttonCode);
bool zf4_is_mouse_button_pressed(const ZF4MouseButtonCode buttonCode);
bool zf4_is_mouse_button_released(const ZF4MouseButtonCode buttonCode);
ZF4Vec2D zf4_get_mouse_pos();

#endif
