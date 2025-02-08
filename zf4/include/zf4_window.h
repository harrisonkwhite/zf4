#pragma once

#include <cassert>
#include <GLFW/glfw3.h>
#include <zf4c.h>

namespace zf4 {
    enum e_window_flags {
        ek_window_flags_none,
        ek_window_flags_hide_cursor = 1 << 0,
        ek_window_flags_resizable = 1 << 1,

        eks_window_flags_cnt
    };

    typedef uint64_t ta_keys_down_bits;
    typedef uint8_t ta_mouse_buttons_down_bits;

    enum e_key_code {
        ek_key_code_null,

        ek_key_code_space,

        ek_key_code_0,
        ek_key_code_1,
        ek_key_code_2,
        ek_key_code_3,
        ek_key_code_4,
        ek_key_code_5,
        ek_key_code_6,
        ek_key_code_7,
        ek_key_code_8,
        ek_key_code_9,

        ek_key_code_a,
        ek_key_code_b,
        ek_key_code_c,
        ek_key_code_d,
        ek_key_code_e,
        ek_key_code_f,
        ek_key_code_g,
        ek_key_code_h,
        ek_key_code_i,
        ek_key_code_j,
        ek_key_code_k,
        ek_key_code_l,
        ek_key_code_m,
        ek_key_code_n,
        ek_key_code_o,
        ek_key_code_p,
        ek_key_code_q,
        ek_key_code_r,
        ek_key_code_s,
        ek_key_code_t,
        ek_key_code_u,
        ek_key_code_v,
        ek_key_code_w,
        ek_key_code_x,
        ek_key_code_y,
        ek_key_code_z,

        ek_key_code_escape,
        ek_key_code_enter,
        ek_key_code_tab,
        ek_key_code_backspace,
        ek_key_code_insert,
        ek_key_code_delete,
        ek_key_code_home,
        ek_key_code_end,
        ek_key_code_page_up,
        ek_key_code_page_down,
        ek_key_code_caps_lock,
        ek_key_code_scroll_lock,
        ek_key_code_num_lock,
        ek_key_code_print_screen,
        ek_key_code_pause,

        ek_key_code_right,
        ek_key_code_left,
        ek_key_code_down,
        ek_key_code_up,

        ek_key_code_f1,
        ek_key_code_f2,
        ek_key_code_f3,
        ek_key_code_f4,
        ek_key_code_f5,
        ek_key_code_f6,
        ek_key_code_f7,
        ek_key_code_f8,
        ek_key_code_f9,
        ek_key_code_f10,
        ek_key_code_f11,
        ek_key_code_f12,

        ek_key_code_left_shift,
        ek_key_code_left_control,
        ek_key_code_left_alt,
        ek_key_code_left_super,
        ek_key_code_right_shift,
        ek_key_code_right_control,
        ek_key_code_right_alt,
        ek_key_code_right_super,

        ek_key_code_menu,

        eks_key_code_cnt
    };

    enum e_mouse_button_code {
        ek_mouse_button_code_null,

        ek_mouse_button_code_left,
        ek_mouse_button_code_right,
        ek_mouse_button_code_middle,

        eks_mouse_button_code_cnt
    };

    struct s_input_state {
        ta_keys_down_bits keys_down;
        ta_mouse_buttons_down_bits mouse_buttons_down;
        s_vec_2d mouse_pos;
    };

    struct s_window {
        GLFWwindow* glfw_window;

        s_vec_2d_i size_cache; // glfwGetWindowSize() is a bit expensive for some reason, so we cache this on resize.

        s_input_state input_state;
        s_input_state input_state_saved;
    };

    bool InitWindow(s_window& window, const s_vec_2d_i size, const char* const title, const e_window_flags flags);
    void CleanWindow(s_window& window);

    inline bool KeyDown(const e_key_code key_code, const s_input_state& input_state) {
        const ta_keys_down_bits key_bit = (ta_keys_down_bits)1 << key_code;
        return input_state.keys_down & key_bit;
    }

    inline bool KeyPressed(const e_key_code key_code, const s_input_state& input_state, const s_input_state& input_state_last) {
        return KeyDown(key_code, input_state) && !KeyDown(key_code, input_state_last);
    }

    inline bool KeyReleased(const e_key_code key_code, const s_input_state& input_state, const s_input_state& input_state_last) {
        return !KeyDown(key_code, input_state) && KeyDown(key_code, input_state_last);
    }

    inline bool MouseButtonDown(const e_mouse_button_code button_code, const s_input_state& input_state) {
        const ta_mouse_buttons_down_bits button_bit = (ta_mouse_buttons_down_bits)1 << button_code;
        return input_state.mouse_buttons_down & button_bit;
    }

    inline bool MouseButtonPressed(const e_mouse_button_code button_code, const s_input_state& input_state, const s_input_state& input_state_last) {
        return MouseButtonDown(button_code, input_state) && !MouseButtonDown(button_code, input_state_last);
    }

    inline bool MouseButtonReleased(const e_mouse_button_code button_code, const s_input_state& input_state, const s_input_state& input_state_last) {
        return !MouseButtonDown(button_code, input_state) && MouseButtonDown(button_code, input_state_last);
    }
}
