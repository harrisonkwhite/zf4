#pragma once

#include <cassert>
#include <GLFW/glfw3.h>
#include <zf4c.h>
#include <zf4_rendering.h>

namespace zf4 {
    enum e_window_flags {
        ek_window_flags_none,
        ek_window_flags_hide_cursor = 1 << 0,
        ek_window_flags_resizable = 1 << 1,

        eks_window_flags_cnt
    };

    using a_keys_down_bits = uint64_t;
    using a_mouse_buttons_down_bits = uint8_t;

    enum e_key_code {
        eks_key_code_null,

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
        ek_key_code_right_shift,
        ek_key_code_right_control,
        ek_key_code_right_alt,

        eks_key_code_cnt
    };

    constexpr s_static_array<const char*, eks_key_code_cnt> g_key_code_names = {
        "",

        "Space",

        "0",
        "1",
        "2",
        "3",
        "4",
        "5",
        "6",
        "7",
        "8",
        "9",

        "A",
        "B",
        "C",
        "D",
        "E",
        "F",
        "G",
        "H",
        "I",
        "J",
        "K",
        "L",
        "M",
        "N",
        "O",
        "P",
        "Q",
        "R",
        "S",
        "T",
        "U",
        "V",
        "W",
        "X",
        "Y",
        "Z",

        "Escape",
        "Enter",
        "Tab",

        "Right",
        "Left",
        "Down",
        "Up",

        "F1",
        "F2",
        "F3",
        "F4",
        "F5",
        "F6",
        "F7",
        "F8",
        "F9",
        "F10",
        "F11",
        "F12",

        "Left Shift",
        "Left Control",
        "Left Alt",
        "Right Shift",
        "Right Control",
        "Right Alt"
    };

    enum e_mouse_button_code {
        ek_mouse_button_code_null,

        ek_mouse_button_code_left,
        ek_mouse_button_code_right,
        ek_mouse_button_code_middle,

        eks_mouse_button_code_cnt
    };

    constexpr s_static_array<const char*, eks_mouse_button_code_cnt> g_mouse_button_code_names = {
        "",

        "Left Mouse Button",
        "Right Mouse Button",
        "Middle Mouse Button"
    };

    struct s_input_state {
        a_keys_down_bits keys_down;
        a_mouse_buttons_down_bits mouse_buttons_down;
        s_vec_2d mouse_pos;
    };

    struct s_window {
        GLFWwindow* glfw_window;

        s_vec_2d_i size_default;
        s_vec_2d_i size_min;

        s_input_state input_state;
        s_input_state input_state_saved;
    };

    bool InitWindow(s_window& window, const bool begin_fullscreen, const s_vec_2d_i size_default, const s_vec_2d_i size_min, const char* const title, const e_window_flags flags);
    void CleanWindow(s_window& window);
    bool ResizeWindow(const s_window& window, const s_vec_2d_i size, rendering::s_pers_render_data& pers_render_data);
    bool SetFullscreen(s_window& window, const bool fullscreen, rendering::s_pers_render_data& pers_render_data);
    bool ProcWindowResize(const s_window& window, rendering::s_pers_render_data& pers_render_data);

    inline s_vec_2d_i WindowSize(const s_window& window) {
        s_vec_2d_i size;
        glfwGetWindowSize(window.glfw_window, &size.x, &size.y);
        return size;
    }

    inline bool InFullscreen(const s_window& window) {
        return glfwGetWindowMonitor(window.glfw_window);
    }

    inline bool KeyDown(const e_key_code key_code, const s_input_state& input_state) {
        const a_keys_down_bits key_bit = static_cast<a_keys_down_bits>(1) << key_code;
        return input_state.keys_down & key_bit;
    }

    inline bool KeyPressed(const e_key_code key_code, const s_input_state& input_state, const s_input_state& input_state_last) {
        return KeyDown(key_code, input_state) && !KeyDown(key_code, input_state_last);
    }

    inline bool KeyReleased(const e_key_code key_code, const s_input_state& input_state, const s_input_state& input_state_last) {
        return !KeyDown(key_code, input_state) && KeyDown(key_code, input_state_last);
    }

    inline bool MouseButtonDown(const e_mouse_button_code button_code, const s_input_state& input_state) {
        const a_mouse_buttons_down_bits button_bit = static_cast<a_mouse_buttons_down_bits>(1) << button_code;
        return input_state.mouse_buttons_down & button_bit;
    }

    inline bool MouseButtonPressed(const e_mouse_button_code button_code, const s_input_state& input_state, const s_input_state& input_state_last) {
        return MouseButtonDown(button_code, input_state) && !MouseButtonDown(button_code, input_state_last);
    }

    inline bool MouseButtonReleased(const e_mouse_button_code button_code, const s_input_state& input_state, const s_input_state& input_state_last) {
        return !MouseButtonDown(button_code, input_state) && MouseButtonDown(button_code, input_state_last);
    }
}
