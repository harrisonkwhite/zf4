#pragma once

#include <zf4_mem.h>
#include <zf4_graphics.h>
#include <zf4_audio.h>

namespace zf4 {
    constexpr int g_gl_version_major = 4;
    constexpr int g_gl_version_minor = 3;

    enum e_window_flags {
        ek_window_flags_none,
        ek_window_flags_hide_cursor = 1 << 0,
        ek_window_flags_resizable = 1 << 1,

        eks_window_flags_cnt
    };

    using a_keys_down_bits = uint64_t;
    using a_mouse_buttons_down_bits = uint8_t;

    enum e_key_code {
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
        ek_mouse_button_code_left,
        ek_mouse_button_code_right,
        ek_mouse_button_code_middle,

        eks_mouse_button_code_cnt
    };

    constexpr s_static_array<const char*, eks_mouse_button_code_cnt> g_mouse_button_code_names = {
        "Left Mouse Button",
        "Right Mouse Button",
        "Middle Mouse Button"
    };

    struct s_window_state {
        s_vec_2d_i size;
        bool fullscreen = false;
    };

    struct s_input_state {
        a_keys_down_bits keys_down = 0;
        a_mouse_buttons_down_bits mouse_buttons_down = 0;
        s_vec_2d mouse_pos;
    };

    struct s_game_init_func_data {
        s_mem_arena& perm_mem_arena;
        const s_window_state& window_state_cache;
        const s_input_state& input_state;
    };

    struct s_game_tick_func_data {
        s_mem_arena& perm_mem_arena;
        const s_window_state& window_state_cache;
        s_window_state& window_state_ideal; // NOTE: Possibly add to initialisation data too?
        const s_input_state& input_state;
        const s_input_state& input_state_last;
        const double fps;
    };

    enum e_game_tick_func_result {
        ek_game_tick_func_result_continue,
        ek_game_tick_func_result_success_quit,
        ek_game_tick_func_result_error_quit
    };

    using a_game_init_func = bool (*)(const s_game_init_func_data& zf4_data);
    using a_game_tick_func = e_game_tick_func_result(*)(const s_game_tick_func_data& zf4_data);

    struct s_game_info {
        a_game_init_func init_func = nullptr;
        a_game_tick_func tick_func = nullptr;

        int perm_mem_arena_size = MegabytesToBytes(80);

        s_vec_2d_i window_size_default = {1280, 720};
        s_vec_2d_i window_size_min;
        const char* window_title = "";
        enum e_window_flags window_flags = ek_window_flags_none;

        s_array<const char* const> audio_file_paths;

        bool IsValid() const {
            return init_func
                && tick_func
                && perm_mem_arena_size > 0
                && window_size_default.x > 0 && window_size_default.y > 0
                && window_size_min.x > 0 && window_size_min.y > 0
                && window_title;
        }
    };

    bool RunGame(const s_game_info& game_info);

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
