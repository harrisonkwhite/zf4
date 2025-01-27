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
        s_renderer& renderer;
        void* custom_data;
    };

    typedef bool (*ta_game_init_func)(const s_game_ptrs& const game_ptrs);
    typedef bool (*ta_game_tick_func)(const s_game_ptrs& const game_ptrs, const double fps);
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

    typedef void (*ta_game_info_loader)(s_game_info* const info);

    bool RunGame(const ta_game_info_loader info_loader);
}
