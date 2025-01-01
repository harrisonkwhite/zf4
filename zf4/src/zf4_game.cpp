#include <zf4_game.h>

#include <zf4c_mem.h>
#include <zf4_window.h>
#include <zf4_assets.h>
#include <zf4_shader_progs.h>
#include <zf4_renderer.h>
#include <zf4_audio.h>
#include <zf4_rand.h>

namespace zf4 {
    static constexpr int ik_targFPS = 60;
    static constexpr double ik_targTickDur = 1.0 / ik_targFPS; // In seconds.
    static constexpr double ik_targTickDurLimitMult = 8.0; // The multiplier on the target tick duration which produces the maximum acceptable tick duration.

    struct Game {
        ShaderProgs shaderProgs;
        SoundSrcManager sndSrcManager;
        MusicSrcManager musicSrcManager;
        Scene scene;
    };

    static double calc_valid_frame_dur(const double frameTime, const double frameTimeLast) {
        const double dur = frameTime - frameTimeLast;
        return dur >= 0.0 && dur <= ik_targTickDur * ik_targTickDurLimitMult ? dur : 0.0;
    }

    static void run_game(Game* const game, const UserGameInfo* const userInfo) {
        assert(is_zero(game));

        //
        // Initialisation
        //
        log("Initialising...");

        if (!glfwInit()) {
            log_error("Failed to initialise GLFW!");
            return;
        }

        if (!init_window(userInfo->windowInitWidth, userInfo->windowInitHeight, userInfo->windowTitle, userInfo->windowResizable, userInfo->windowHideCursor)) {
            return;
        }

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            log_error("Failed to initialise OpenGL function pointers!");
            return;
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        if (!init_audio_system()) {
            return;
        }

        if (!load_assets()) {
            return;
        }

        load_shader_progs(&game->shaderProgs);

        if (!load_sprites(userInfo->spriteCnt, userInfo->spriteLoader)) {
            return;
        }

        if (!load_component_types(userInfo->componentTypeCnt, userInfo->componentTypeInfoLoader)) {
            return;
        }

        if (!load_scene_types(userInfo->sceneTypeCnt, userInfo->sceneTypeInfoLoader)) {
            return;
        }

        init_rng();

        if (!load_scene(&game->scene, 0)) { // We begin with the first scene.
            return;
        }

        show_window();

        //
        // Main Loop
        //
        double frameTime = glfwGetTime();
        double frameDurAccum = 0.0;

        log("Entering the main loop...");

        while (!window_should_close()) {
            const double frameTimeLast = frameTime;
            frameTime = glfwGetTime();

            const double frameDur = calc_valid_frame_dur(frameTime, frameTimeLast);
            frameDurAccum += frameDur;

            const int tickCnt = (int)(frameDurAccum / ik_targTickDur);

            if (tickCnt > 0) {
                int i = 0;

                do {
                    handle_auto_release_sound_srcs(&game->sndSrcManager);

                    if (!refresh_music_src_bufs(&game->musicSrcManager)) {
                        return;
                    }

                    if (!proc_scene_tick(&game->scene)) {
                        return;
                    }

                    frameDurAccum -= ik_targTickDur;
                    ++i;
                } while (i < tickCnt);

                save_input_state();
            }

            render_all(&game->scene.renderer, &game->shaderProgs);
            swap_window_buffers();

            glfwPollEvents();
        }
    }

    void start_game(const UserGameInfo* const userInfo) {
        Game game = {};

        run_game(&game, userInfo);

        unload_scene(&game.scene);
        unload_scene_types();
        unload_component_types();
        unload_sprites();
        clean_music_srcs(&game.musicSrcManager);
        clean_sound_srcs(&game.sndSrcManager);
        unload_shader_progs(&game.shaderProgs);
        unload_assets();
        clean_audio_system();
        clean_window();
        glfwTerminate();
    }
}
