#ifndef ZF4_WINDOW_H
#define ZF4_WINDOW_H

#include <stdbool.h>
#include <assert.h>
#include <GLFW/glfw3.h>
#include <zf4c.h>

enum window_flags {
    window_flags__none,
    window_flags__hide_cursor = 1 << 0,
    window_flags__resizable = 1 << 1,

    window_flags_cnt
};

typedef uint64_t ta_keys_down_bits;
typedef uint8_t ta_mouse_buttons_down_bits;

enum key_code {
    key_code__null,

    key_code__space,

    key_code__0,
    key_code__1,
    key_code__2,
    key_code__3,
    key_code__4,
    key_code__5,
    key_code__6,
    key_code__7,
    key_code__8,
    key_code__9,

    key_code__a,
    key_code__b,
    key_code__c,
    key_code__d,
    key_code__e,
    key_code__f,
    key_code__g,
    key_code__h,
    key_code__i,
    key_code__j,
    key_code__k,
    key_code__l,
    key_code__m,
    key_code__n,
    key_code__o,
    key_code__p,
    key_code__q,
    key_code__r,
    key_code__s,
    key_code__t,
    key_code__u,
    key_code__v,
    key_code__w,
    key_code__x,
    key_code__y,
    key_code__z,

    key_code__escape,
    key_code__enter,
    key_code__tab,

    key_code__right,
    key_code__left,
    key_code__down,
    key_code__up,

    key_code__f1,
    key_code__f2,
    key_code__f3,
    key_code__f4,
    key_code__f5,
    key_code__f6,
    key_code__f7,
    key_code__f8,
    key_code__f9,
    key_code__f10,
    key_code__f11,
    key_code__f12,

    key_code__left_shift,
    key_code__left_control,
    key_code__left_alt,

    key_code_cnt
};

enum mouse_button_code {
    mouse_button_code__null,

    mouse_button_code__left,
    mouse_button_code__right,
    mouse_button_code__middle,

    mouse_button_code_cnt
};

typedef struct input_state {
    ta_keys_down_bits keys_down;
    ta_mouse_buttons_down_bits mouse_buttons_down;
    s_vec_2d mouse_pos;
} s_input_state;

typedef struct window {
    GLFWwindow* glfw_window;

    s_vec_2d_i size_cache; // glfwGetWindowSize() is a bit expensive for some reason, so we cache this.

    s_input_state input_state;
    s_input_state input_state_saved;
} s_window;

bool InitWindow(s_window* const window, const s_vec_2d_i size, const char* const title, const enum window_flags flags);
void CleanWindow(s_window* const window);

inline bool IsInputStateValid(const s_input_state* const input_state) {
    assert(input_state);
    return input_state->keys_down < ((ta_keys_down_bits)1 << key_code_cnt)
        && input_state->mouse_buttons_down < ((ta_mouse_buttons_down_bits)1 << mouse_button_code_cnt);
}

inline bool IsWindowValid(const s_window* const window) {
    assert(window);
    return IsInputStateValid(&window->input_state)
        && IsInputStateValid(&window->input_state_saved);
}

inline bool KeyDown(const enum key_code key_code, const s_input_state* const input_state) {
    assert(key_code >= 0 && key_code < key_code_cnt);
    assert(IsInputStateValid(input_state));

    const ta_keys_down_bits key_bit = (ta_keys_down_bits)1 << key_code;
    return input_state->keys_down & key_bit;
}

inline bool KeyPressed(const enum key_code key_code, const s_input_state* const input_state, const s_input_state* const input_state_last) {
    assert(key_code >= 0 && key_code < key_code_cnt);
    assert(IsInputStateValid(input_state));
    assert(IsInputStateValid(input_state_last));

    return KeyDown(key_code, input_state) && !KeyDown(key_code, input_state_last);
}

inline bool KeyReleased(const enum key_code key_code, const s_input_state* const input_state, const s_input_state* const input_state_last) {
    assert(key_code >= 0 && key_code < key_code_cnt);
    assert(IsInputStateValid(input_state));
    assert(IsInputStateValid(input_state_last));

    return !KeyDown(key_code, input_state) && KeyDown(key_code, input_state_last);
}

inline bool MouseButtonDown(const enum mouse_button_code button_code, const s_input_state* const input_state) {
    assert(button_code >= 0 && button_code < mouse_button_code_cnt);
    assert(IsInputStateValid(input_state));

    const ta_mouse_buttons_down_bits button_bit = (ta_mouse_buttons_down_bits)1 << button_code;
    return input_state->mouse_buttons_down & button_bit;
}

inline bool MouseButtonPressed(const enum mouse_button_code button_code, const s_input_state* const input_state, const s_input_state* const input_state_last) {
    assert(button_code >= 0 && button_code < mouse_button_code_cnt);
    assert(IsInputStateValid(input_state));
    assert(IsInputStateValid(input_state_last));

    return MouseButtonDown(button_code, input_state) && !MouseButtonDown(button_code, input_state_last);
}

inline bool MouseButtonReleased(const enum mouse_button_code button_code, const s_input_state* const input_state, const s_input_state* const input_state_last) {
    assert(button_code >= 0 && button_code < mouse_button_code_cnt);
    assert(IsInputStateValid(input_state));
    assert(IsInputStateValid(input_state_last));

    return !MouseButtonDown(button_code, input_state) && MouseButtonDown(button_code, input_state_last);
}

#endif
