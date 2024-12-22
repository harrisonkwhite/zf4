#ifndef ZF4_GAME_H
#define ZF4_GAME_H

#include <zf4c_mem.h>
#include <zf4_window.h>
#include <zf4_assets.h>
#include <zf4_shader_progs.h>

typedef struct {
    int windowInitWidth;
    int windowInitHeight;
    const char* windowTitle;
    bool windowResizable;
    bool windowHideCursor;
} ZF4UserGameInfo;

void zf4_run_game(const ZF4UserGameInfo* const userInfo);

#endif
