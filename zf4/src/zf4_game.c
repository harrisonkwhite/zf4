#include <zf4_game.h>

#include <threads.h>

#include <zf4c_mem.h>
#include <zf4_window.h>
#include <zf4_assets.h>
#include <zf4_shader_progs.h>
#include <zf4_renderer.h>
#include <zf4_audio.h>

#define MEM_ARENA_SIZE ZF4_MEGABYTES(32)

#define TARG_FPS 60
#define TARG_TICK_DUR (1.0 / TARG_FPS) // In seconds.
#define TARG_TICK_DUR_LIMIT_MULT 8.0 // The multiplier on the target tick duration which produces the maximum acceptable tick duration.

typedef struct {
    ZF4MemArena memArena;
    ZF4Assets assets;
    ZF4ShaderProgs shaderProgs;
    ZF4SoundSrcManager sndSrcManager;
    ZF4MusicSrcManager musicSrcManager;
    ZF4SceneManager sceneManager;
} Game;

static double calc_valid_frame_dur(const double frameTime, const double frameTimeLast) {
    const double dur = frameTime - frameTimeLast;
    return dur >= 0.0 && dur <= TARG_TICK_DUR * TARG_TICK_DUR_LIMIT_MULT ? dur : 0.0;
}

static void run_game(Game* const game, const ZF4UserGameInfo* const userInfo) {
    assert(zf4_is_zero(game, sizeof(*game)));

    //
    // Initialisation
    //
    zf4_log("Initialising...");

    if (!zf4_init_mem_arena(&game->memArena, MEM_ARENA_SIZE)) {
        zf4_log_error("Failed to initialise game memory arena!");
        return;
    }

    if (!glfwInit()) {
        zf4_log_error("Failed to initialise GLFW!");
        return;
    }

    if (!zf4_init_window(userInfo->windowInitWidth, userInfo->windowInitHeight, userInfo->windowTitle, userInfo->windowResizable, userInfo->windowHideCursor)) {
        return;
    }

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        zf4_log_error("Failed to initialise OpenGL function pointers!");
        return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (!zf4_init_audio_system()) {
        return;
    }

    if (!zf4_load_assets(&game->assets, &game->memArena)) {
        return;
    }

    zf4_load_shader_progs(&game->shaderProgs);

    if (!zf4_load_anim_types(userInfo->animTypeCnt, userInfo->animTypeLoader)) {
        return;
    }

    const ZF4GamePtrs gamePtrs = {
        .assets = &game->assets,
        .sndSrcManager = &game->sndSrcManager,
        .musicSrcManager = &game->musicSrcManager
    };

    game->sceneManager.typeInfoLoader = userInfo->sceneTypeInfoLoader;

    if (!zf4_load_scene_of_type(&game->sceneManager, 0, &gamePtrs)) { // We begin with the first scene index.
        return;
    }

    zf4_show_window();

    //
    // Main Loop
    //
    double frameTime = glfwGetTime();
    double frameDurAccum = 0.0;

    zf4_log("Entering the main loop...");

    while (!zf4_window_should_close()) {
        const double frameTimeLast = frameTime;
        frameTime = glfwGetTime();

        const double frameDur = calc_valid_frame_dur(frameTime, frameTimeLast);
        frameDurAccum += frameDur;

        const int tickCnt = frameDurAccum / TARG_TICK_DUR;

        if (tickCnt > 0) {
            int i = 0;

            do {
                zf4_handle_auto_release_sound_srcs(&game->sndSrcManager);

                if (!zf4_refresh_music_src_bufs(&game->musicSrcManager, &game->assets.music)) {
                    return;
                }

                if (!zf4_proc_scene_tick(&game->sceneManager, &gamePtrs)) {
                    return;
                }

                frameDurAccum -= TARG_TICK_DUR;
                ++i;
            } while (i < tickCnt);

            zf4_save_input_state();
        }

        zf4_render_all(&game->sceneManager.scene.renderer, &game->shaderProgs, &game->assets);
        zf4_swap_window_buffers();

        glfwPollEvents();
    }
}

void zf4_start_game(const ZF4UserGameInfo* const userInfo) {
    Game game = {0};

    run_game(&game, userInfo);

    zf4_unload_scene(&game.sceneManager.scene);
    zf4_unload_anim_types();
    zf4_clean_music_srcs(&game.musicSrcManager);
    zf4_clean_sound_srcs(&game.sndSrcManager);
    zf4_unload_shader_progs(&game.shaderProgs);
    zf4_unload_assets(&game.assets);
    zf4_clean_audio_system();
    zf4_clean_window();
    glfwTerminate();
    zf4_clean_mem_arena(&game.memArena);
}
