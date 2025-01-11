#include <zf4_game.h>

#include <zf4c_mem.h>
#include <zf4_window.h>
#include <zf4_assets.h>
#include <zf4_renderer.h>
#include <zf4_audio.h>
#include <zf4_rand.h>

namespace zf4 {
    static constexpr int ik_targFPS = 60;
    static constexpr double ik_targTickDur = 1.0 / ik_targFPS; // In seconds.

    struct Game {
        MemArena memArena;
        Assets assets;
        InternalShaderProgs internalShaderProgs;
        SoundSrcManager sndSrcManager;
        MusicSrcManager musicSrcManager;
        Array<Sprite> sprites;
        Array<SceneTypeInfo> sceneTypeInfos;
        Scene scene;
    };

    Game i_game;

    static void run_game(const UserGameInfo* const userInfo) {
        assert(is_zero(&i_game));

        //
        // Initialisation
        //
        log("Initialising...");

        if (!i_game.memArena.init(megabytes_to_bytes(4))) {
            log_error("Failed to initialise the memory arena!");
            return;
        }

        if (!glfwInit()) {
            log_error("Failed to initialise GLFW!");
            return;
        }

        if (!Window::init(userInfo->windowInitWidth, userInfo->windowInitHeight, userInfo->windowTitle, userInfo->windowResizable, userInfo->windowHideCursor)) {
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

        if (!i_game.assets.load(&i_game.memArena)) {
            return;
        }

        i_game.internalShaderProgs = load_internal_shader_progs();

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

        const GamePtrs gamePtrs = {
            .soundSrcManager = &i_game.sndSrcManager,
            .musicSrcManager = &i_game.musicSrcManager
        };

        if (!load_scene(&i_game.scene, 0, i_game.sceneTypeInfos, gamePtrs)) { // We begin with the first scene.
            return;
        }

        Window::show();

        //
        // Main Loop
        //
        double frameTime = glfwGetTime();
        double frameDurAccum = 0.0;

        log("Entering the main loop...");

        while (!Window::should_close()) {
            const double frameTimeLast = frameTime;
            frameTime = glfwGetTime();

            const double frameDur = frameTime - frameTimeLast;
            frameDurAccum += frameDur;

            if (frameDurAccum >= ik_targTickDur) {
                do {
                    handle_auto_release_sound_srcs(&i_game.sndSrcManager);

                    if (!refresh_music_src_bufs(&i_game.musicSrcManager, i_game.assets)) {
                        return;
                    }

                    if (!proc_scene_tick(&i_game.scene, i_game.sceneTypeInfos, gamePtrs)) {
                        return;
                    }

                    frameDurAccum -= ik_targTickDur;
                } while (frameDurAccum >= ik_targTickDur);

                Window::save_input_state();
            }

            if (!i_game.scene.renderer.render(i_game.internalShaderProgs, i_game.assets, &i_game.scene.scratchSpace)) {
                return;
            }

            Window::swap_buffers();

            const zf4::Vec2DI windowSizePrepoll = Window::get_size();

            glfwPollEvents();

            // Check for and process window resize.
            if (Window::get_size().x != windowSizePrepoll.x || Window::get_size().y != windowSizePrepoll.y) {
                log("Processing window resize...");
                glViewport(0, 0, Window::get_size().x, Window::get_size().y);
                i_game.scene.renderer.resize_surfaces();
            }
        }
    }

    void start_game(const UserGameInfo* const userInfo) {
        assert(zf4::is_zero(&i_game));

        run_game(userInfo);

        unload_scene(&i_game.scene);
        unload_component_types();
        clean_music_srcs(&i_game.musicSrcManager);
        clean_sound_srcs(&i_game.sndSrcManager);
        i_game.assets.clean();
        clean_audio_system();
        Window::clean();
        glfwTerminate();
        i_game.memArena.clean();

        i_game = {};
    }

    const Array<Sprite>& get_game_sprites() {
        return i_game.sprites;
    }
}
