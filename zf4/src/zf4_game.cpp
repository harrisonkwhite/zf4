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

    struct Game {
        MemArena memArena;

        ShaderProgs shaderProgs;

        Renderer renderer;

        SoundSrcManager sndSrcManager;

        MusicSrcManager musicSrcManager;

        Array<Sprite> sprites;

        Scene scene;

        Array<SceneTypeInfo> sceneTypeInfos;
    };

    Game i_game;

    static void run_game(const UserGameInfo* const userInfo) {
        assert(is_zero(&i_game));

        //
        // Initialisation
        //
        log("Initialising...");

        if (!i_game.memArena.init(megabytes_to_bytes(256))) {
            log_error("Failed to initialise the memory arena!");
            return;
        }

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

        load_shader_progs(&i_game.shaderProgs);

        if (!i_game.renderer.init(&i_game.memArena)) {
            log_error("Failed to initialise the renderer!");
            return;
        }

        if (!userInfo->spritesLoader(&i_game.sprites, &i_game.memArena)) {
            log_error("Failed to load sprites!");
            return;
        }

        if (!load_component_types(userInfo->componentTypeCnt, userInfo->componentTypeInfoLoader)) {
            return;
        }

        if (!userInfo->sceneTypeInfosLoader(&i_game.sceneTypeInfos, &i_game.memArena)) {
            return;
        }

        init_rng();

        const GamePtrs gamePtrs = {&i_game.renderer}; // TEMP

        if (!load_scene(&i_game.scene, 0, i_game.sceneTypeInfos, gamePtrs)) { // We begin with the first scene.
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

            const double frameDur = frameTime - frameTimeLast;
            frameDurAccum += frameDur;

            if (frameDurAccum >= ik_targTickDur) {
                do {
                    i_game.renderer.begin_writeup();

                    handle_auto_release_sound_srcs(&i_game.sndSrcManager);

                    if (!refresh_music_src_bufs(&i_game.musicSrcManager)) {
                        return;
                    }

                    if (!proc_scene_tick(&i_game.scene, i_game.sceneTypeInfos, gamePtrs)) {
                        return;
                    }

                    i_game.renderer.end_writeup();

                    frameDurAccum -= ik_targTickDur;
                } while (frameDurAccum >= ik_targTickDur);

                save_input_state();
            }

            const SceneTypeInfo* const sceneTypeInfo = &i_game.sceneTypeInfos[i_game.scene.typeIndex];
            i_game.renderer.render(sceneTypeInfo->bgColor, &i_game.shaderProgs);
            swap_window_buffers();

            glfwPollEvents();
        }
    }

    void start_game(const UserGameInfo* const userInfo) {
        assert(zf4::is_zero(&i_game));

        run_game(userInfo);

        unload_scene(&i_game.scene);
        unload_component_types();
        clean_music_srcs(&i_game.musicSrcManager);
        clean_sound_srcs(&i_game.sndSrcManager);
        i_game.renderer.clean();
        unload_shader_progs(&i_game.shaderProgs);
        unload_assets();
        clean_audio_system();
        clean_window();
        glfwTerminate();
        i_game.memArena.clean();

        i_game = {};
    }

    const Array<Sprite>& get_game_sprites() {
        return i_game.sprites;
    }
}
