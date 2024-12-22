#include <zf4_game.h>

#define MEM_ARENA_SIZE ZF4_MEGABYTES(32)

#define TARG_FPS 60
#define TARG_TICK_DUR (1.0 / TARG_FPS)
#define TARG_TICK_DUR_LIMIT_MULT 8.0

typedef struct {
    ZF4MemArena memArena;
    ZF4Assets assets;
    ZF4ShaderProgs shaderProgs;
    ZF4SceneManager sceneManager;
} Game;

static void clean_game(Game* const game) {
    zf4_unload_scene(&game->sceneManager.scene);
    zf4_unload_shader_progs(&game->shaderProgs);
    zf4_unload_assets(&game->assets);
    zf4_clean_window();
    glfwTerminate();
    zf4_clean_mem_arena(&game->memArena);
}

static double calc_valid_frame_dur(const double frameTime, const double frameTimeLast) {
    const double dur = frameTime - frameTimeLast;
    return dur >= 0.0 && dur <= TARG_TICK_DUR * TARG_TICK_DUR_LIMIT_MULT ? dur : 0.0;
}

void zf4_run_game(const ZF4UserGameInfo* const userInfo) {
    Game game = {0};

    if (!zf4_init_mem_arena(&game.memArena, MEM_ARENA_SIZE)) {
        clean_game(&game);
        return;
    }

    if (!glfwInit()) {
        clean_game(&game);
        return;
    }

    if (!zf4_init_window(userInfo->windowInitWidth, userInfo->windowInitHeight, userInfo->windowTitle, userInfo->windowResizable, userInfo->windowHideCursor)) {
        clean_game(&game);
        return;
    }

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        zf4_log_error("Failed to initialise OpenGL function pointers!");
        clean_game(&game);
        return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (!zf4_load_assets(&game.assets, &game.memArena)) {
        clean_game(&game);
        return;
    }

    zf4_load_shader_progs(&game.shaderProgs);

    game.sceneManager.typeInfoLoader = userInfo->sceneTypeInfoLoader;

    if (!zf4_load_scene_of_type(&game.sceneManager, 0)) {
        clean_game(&game);
        return;
    }

    zf4_show_window();

    double frameTime = glfwGetTime();
    double frameDurAccum = 0.0;

    while (!zf4_window_should_close()) {
        const double frameTimeLast = frameTime;
        frameTime = glfwGetTime();

        const double frameDur = calc_valid_frame_dur(frameTime, frameTimeLast);
        frameDurAccum += frameDur;

        const int tickCnt = frameDurAccum / TARG_TICK_DUR;

        if (tickCnt > 0) {
            int i = 0;

            do {
                if (!zf4_proc_scene_tick(&game.sceneManager)) {
                    clean_game(&game);
                    return;
                }

                frameDurAccum -= TARG_TICK_DUR;
                ++i;
            } while (i < tickCnt);

            zf4_save_input_state();
        }

        zf4_render_all(&game.sceneManager.scene.renderer, &game.shaderProgs, &game.assets);
        zf4_swap_window_buffers();

        glfwPollEvents();
    }

    clean_game(&game);
}
