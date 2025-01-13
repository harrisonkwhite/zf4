#include <zf4_game.h>

#include <zf4c_mem.h>
#include <zf4_rand.h>

namespace zf4 {
    static constexpr int ik_targFPS = 60;
    static constexpr double ik_targTickDurSecs = 1.0 / ik_targFPS;

    struct Game {
        MemArena memArena;
        Assets assets;
        InternalShaderProgs internalShaderProgs;
        Renderer renderer;
        SoundSrcManager sndSrcManager;
        MusicSrcManager musicSrcManager;
        Array<Sprite> sprites;
        Array<ComponentType> compTypes;
        Array<SceneType> sceneTypes;
        Scene scene;
    };

    static bool exec_game_init_and_main_loop(Game* const game, const UserGameInfo& userInfo) {
        assert(is_zero(game));

        //
        // Initialisation
        //
        log("Initialising...");

        // Set up memory arenas.
        if (!game->memArena.init(userInfo.memArenaSize)) {
            log_error("Failed to initialise the memory arena!");
            return false;
        }

        // Set up GLFW and the window.
        if (!glfwInit()) {
            log_error("Failed to initialise GLFW!");
            return false;
        }

        if (!Window::init(userInfo.windowInitSize.x, userInfo.windowInitSize.y, userInfo.windowTitle, userInfo.windowFlags)) {
            return false;
        }

        // Set up OpenGL.
        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
            log_error("Failed to initialise OpenGL function pointers!");
            return false;
        }

        // Set up the audio system (which uses OpenAL).
        if (!init_audio_system()) {
            return false;
        }

        // Load assets.
        if (!game->assets.load(&game->memArena)) {
            return false;
        }

        game->internalShaderProgs = load_internal_shader_progs();

        // Set up the renderer.
        if (!game->renderer.init(&game->memArena)) {
            return false;
        }

        // Initialise sprites.
        if (!game->sprites.init(&game->memArena, userInfo.spriteCnt)) {
            return false;
        }

        for (int i = 0; i < userInfo.spriteCnt; ++i) {
            if (!userInfo.spriteInitializer(&game->sprites[i], &game->memArena, i, game->assets)) {
                return false;
            }
        }

        // Initialise component types.
        if (!game->compTypes.init(&game->memArena, userInfo.componentTypeCnt)) {
            return false;
        }

        for (int i = 0; i < userInfo.componentTypeCnt; ++i) {
            userInfo.componentTypeInitializer(&game->compTypes[i], i);
        }

        // Initialise scene types.
        if (!game->sceneTypes.init(&game->memArena, userInfo.sceneTypeCnt)) {
            return false;
        }

        for (int i = 0; i < userInfo.sceneTypeCnt; ++i) {
            userInfo.sceneTypeInitializer(&game->sceneTypes[i], i);
        }

        // Set up the random number generator.
        init_rng();

        // Define a struct of pointers to game data to pass to the user-defined game functions.
        const GamePtrs gamePtrs = {
            .assets = &game->assets,
            .renderer = &game->renderer,
            .soundSrcManager = &game->sndSrcManager,
            .musicSrcManager = &game->musicSrcManager,
            .sprites = game->sprites,
            .compTypes = game->compTypes,
            .sceneTypes = game->sceneTypes
        };

        // Call the user-defined initialisation function.
        userInfo.init(gamePtrs);

        // Load the first scene.
        if (!load_scene(&game->scene, 0, gamePtrs)) {
            return false;
        }

        // Now that everything is initialised, show the window.
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

            if (frameDurAccum >= ik_targTickDurSecs) {
                do {
                    handle_auto_release_sound_srcs(&game->sndSrcManager);

                    if (!refresh_music_src_bufs(&game->musicSrcManager, game->assets)) {
                        return false;
                    }

                    if (!proc_scene_tick(&game->scene, gamePtrs)) {
                        return false;
                    }

                    frameDurAccum -= ik_targTickDurSecs;
                } while (frameDurAccum >= ik_targTickDurSecs);

                Window::save_input_state();
            }

            if (!game->renderer.render(game->internalShaderProgs, game->assets, &game->scene.scratchSpace)) {
                return false;
            }

            Window::swap_buffers();

            const zf4::Vec2DI windowSizePrepoll = Window::get_size();

            glfwPollEvents();

            // Check for and process window resize.
            if (Window::get_size().x != windowSizePrepoll.x || Window::get_size().y != windowSizePrepoll.y) {
                log("Processing window resize...");
                glViewport(0, 0, Window::get_size().x, Window::get_size().y);
                game->renderer.resize_surfaces();
            }
        }

        return true;
    }

    bool run_game(const UserGameInfoInitializer userInfoInitializer) {
        //
        // Loading User Game Information
        //

        // Initialise the user information struct with some default game settings.
        UserGameInfo userInfo = {
            .memArenaSize = megabytes_to_bytes(64),

            .windowInitSize = {1280, 720}, // NOTE: Could use a function pointer here instead, to allow for the size to be dynamically determined based on display size for example.
            .windowTitle = "ZF4 Game"
        };

        // Run the initialiser function, which could overwrite some of the above defaults.
        userInfoInitializer(&userInfo);

        // Verify that things have been set correctly.
        assert(userInfo.init);
        assert(userInfo.cleanup);

        assert(userInfo.spriteInitializer);

        if (userInfo.componentTypeCnt > 0) {
            assert(userInfo.componentTypeInitializer);
        } else {
            assert(userInfo.componentTypeCnt == 0);
        }

        assert(userInfo.sceneTypeInitializer);

        //
        // Running the Game
        //
        Game game = {};
        const bool success = exec_game_init_and_main_loop(&game, userInfo);

        //
        // Cleanup
        //
        userInfo.cleanup();
        unload_scene(&game.scene);
        clean_music_srcs(&game.musicSrcManager);
        clean_sound_srcs(&game.sndSrcManager);
        game.assets.clean();
        clean_audio_system();
        Window::clean();
        glfwTerminate();
        game.memArena.clean();

        return success;
    }
}
