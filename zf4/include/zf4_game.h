#pragma once

#include <zf4c.h>
#include <zf4_window.h>
#include <zf4_assets.h>
#include <zf4_rendering.h>

namespace zf4 {
    constexpr int g_gl_version_major = 4;
    constexpr int g_gl_version_minor = 3;

    struct s_game_ptrs {
        s_mem_arena& perm_mem_arena;
        s_mem_arena& temp_mem_arena;
        const s_window& window;
        const s_assets& assets;
        s_pers_render_data& pers_render_data;
        void* custom_data;
    };

    enum e_game_tick_func_result {
        ek_game_tick_func_result_continue,
        ek_game_tick_func_result_success_quit,
        ek_game_tick_func_result_error_quit
    };

    typedef bool (*ta_game_init_func)(const s_game_ptrs& const game_ptrs);
    typedef e_game_tick_func_result (*ta_game_tick_func)(const s_game_ptrs& const game_ptrs, const double fps);
    typedef bool (*ta_game_draw_func)(s_draw_phase_state& const draw_phase_state, const s_game_ptrs& const game_ptrs, const double fps);

    // TODO: Figure out how to do a cleanup user function.

    struct s_game_info {
        ta_game_init_func init_func;
        ta_game_tick_func tick_func;
        ta_game_draw_func draw_func;

        int perm_mem_arena_size;
        int temp_mem_arena_size;

        const s_vec_2d_i window_init_size;
        const char* window_title;
        enum e_window_flags window_flags;

        int custom_data_size;
        int custom_data_alignment;
    };

    using a_game_info_loader = void (*)(s_game_info& info);

    bool RunGame(const a_game_info_loader info_loader);
}
