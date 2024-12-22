#include <zf4_game.h>

#define MEM_ARENA_SIZE ZF4_MEGABYTES(32)

typedef struct {
    ZF4MemArena arena;
} GameMem;

static void clean_game(GameMem* const mem) {
    zf4_clean_window();
    zf4_clean_mem_arena(&mem->arena);
}

void zf4_run_game(const ZF4UserGameInfo* const userInfo) {
    GameMem mem = {0};

    if (!zf4_init_mem_arena(&mem.arena, MEM_ARENA_SIZE)) {
        clean_game(&mem);
        return;
    }

    if (!zf4_init_window(userInfo->windowInitWidth, userInfo->windowInitHeight, userInfo->windowTitle, userInfo->windowResizable, userInfo->windowHideCursor)) {
        clean_game(&mem);
        return;
    }

    while (!zf4_window_should_close()) {
        zf4_swap_window_buffers();
        glfwPollEvents();
    }

    clean_game(&mem);
}
