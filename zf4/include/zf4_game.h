#ifndef ZF4_GAME_H
#define ZF4_GAME_H

#include <zf4c.h>
#include <zf4_window.h>
#include <zf4_assets.h>
#include <zf4_rendering.h>

#define ZF4_GL_VERSION_MAJOR 4
#define ZF4_GL_VERSION_MINOR 3

typedef struct {
    s_mem_arena* perm_mem_arena;
    s_mem_arena* temp_mem_arena;
    const s_window* window;
    const s_assets* assets;
    s_renderer* renderer;
    void* custom_data;
} s_game_ptrs;

typedef bool (*ta_game_init_func)(const s_game_ptrs* const game_ptrs);
typedef bool (*ta_game_tick_func)(const s_game_ptrs* const game_ptrs, const double fps);
typedef bool (*ta_game_draw_func)(s_draw_phase_state* const draw_phase_state, const s_game_ptrs* const game_ptrs, const double fps);

// TODO: Figure out how to do a cleanup user function.

typedef struct {
    ta_game_init_func init_func;
    ta_game_tick_func tick_func;
    ta_game_draw_func draw_func;

    int perm_mem_arena_size;
    int temp_mem_arena_size;

    const s_vec_2d_i window_init_size;
    const char* window_title;
    enum window_flags window_flags;

    int custom_data_size;
    int custom_data_alignment;
} s_game_info;

typedef void (*ta_game_info_loader)(s_game_info* const info);

bool RunGame(const ta_game_info_loader info_loader);

#endif
