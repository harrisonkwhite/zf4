#include <zf4_game.h>

#include <zf4c_mem.h>
#include <zf4_rand.h>

namespace zf4 {
    static constexpr int ik_targFPS = 60;
    static constexpr double ik_targTickDurSecs = 1.0 / ik_targFPS;

    struct Game {
        MemArena permMemArena;
        MemArena tempMemArena;
        Window window;
        Assets assets;
        InternalShaderProgs internalShaderProgs;
        Renderer renderer;
        SoundSrcManager sndSrcManager;
        MusicSrcManager musicSrcManager;
        Array<Sprite> sprites;
        Array<ComponentType> compTypes;
        EntityManager entManager;
    };

    static bool exec_game_init_and_main_loop(Game& game, const GameInfo& gameInfo) {
        assert(is_zero(&game));

        //
        // Initialisation
        //
        log("Initialising...");

        // Set up memory arenas.
        if (!game.permMemArena.init(gameInfo.permMemArenaSize)) {
            log_error("Failed to initialise the permanent memory arena!");
            return false;
        }

        if (!game.tempMemArena.init(gameInfo.tempMemArenaSize)) {
            log_error("Failed to initialise the temporary memory arena!");
            return false;
        }

        // Set up GLFW and the window.
        if (!glfwInit()) {
            log_error("Failed to initialise GLFW!");
            return false;
        }

        if (!init_window(game.window, gameInfo.windowInitSize, gameInfo.windowTitle, gameInfo.windowFlags)) {
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
        if (!game.assets.load(game.permMemArena)) {
            return false;
        }

        game.internalShaderProgs = load_internal_shader_progs();

        // Set up the renderer.
        if (!game.renderer.init(game.permMemArena, gameInfo.renderBatchLimit, gameInfo.renderBatchLife)) {
            return false;
        }

        // Initialise sprites.
        if (gameInfo.spriteCnt > 0) {
            if (!game.sprites.init(game.permMemArena, gameInfo.spriteCnt)) {
                return false;
            }

            for (int i = 0; i < gameInfo.spriteCnt; ++i) {
                if (!gameInfo.spriteInitializer(game.sprites.get(i), game.permMemArena, i, game.assets)) {
                    return false;
                }
            }
        }

        // Initialise component types.
        if (gameInfo.componentTypeCnt > 0) {
            if (!game.compTypes.init(game.permMemArena, gameInfo.componentTypeCnt)) {
                return false;
            }

            for (int i = 0; i < gameInfo.componentTypeCnt; ++i) {
                gameInfo.componentTypeInitializer(game.compTypes.get(i), i);
            }
        }

        // Initialise the entity manager.
        if (!game.entManager.init(game.permMemArena, gameInfo.entLimit, game.compTypes)) {
            return false;
        }

        // Set up the random number generator.
        init_rng();

        // Define a struct of pointers to game data to pass to the user-defined game functions.
        const GamePtrs gamePtrs = {
            .permMemArena = game.permMemArena,
            .tempMemArena = game.tempMemArena,
            .window = game.window,
            .assets = game.assets,
            .renderer = game.renderer,
            .soundSrcManager = game.sndSrcManager,
            .musicSrcManager = game.musicSrcManager,
            .sprites = game.sprites,
            .compTypes = game.compTypes,
            .entManager = game.entManager
        };

        // Call the user-defined initialisation function.
        if (!gameInfo.init(gamePtrs)) {
            return false;
        }

        // Now that everything is initialised, show the window.
        glfwShowWindow(game.window.glfwWindow);

        //
        // Main Loop
        //
        double frameTime = glfwGetTime();
        double frameDurAccum = ik_targTickDurSecs; // Make sure that we begin with a tick.

        log("Entering the main loop...");

        while (!glfwWindowShouldClose(game.window.glfwWindow)) {
            game.tempMemArena.reset();

            const double frameTimeLast = frameTime;
            frameTime = glfwGetTime();

            const double frameDur = frameTime - frameTimeLast;
            frameDurAccum += frameDur;

            if (frameDurAccum >= ik_targTickDurSecs) {
                do {
                    // Execute a tick.
                    handle_auto_release_sound_srcs(&game.sndSrcManager);

                    if (!refresh_music_src_bufs(&game.musicSrcManager, game.assets)) {
                        return false;
                    }

                    if (!gameInfo.tick(gamePtrs)) {
                        return false;
                    }

                    frameDurAccum -= ik_targTickDurSecs;
                } while (frameDurAccum >= ik_targTickDurSecs);

                game.window.inputStateSaved = game.window.inputState;

                // Execute render.
                game.renderer.begin_submission_phase();

                if (!gameInfo.draw(gamePtrs)) {
                    return false;
                }

                game.renderer.end_submission_phase();

                if (!game.renderer.render(game.internalShaderProgs, game.assets, game.window.size, game.tempMemArena)) {
                    return false;
                }

                glfwSwapBuffers(game.window.glfwWindow);
            }

            const zf4::Vec2DI windowSizePrepoll = game.window.size;

            glfwPollEvents();

            // Check for and process window resize.
            if (game.window.size.x != windowSizePrepoll.x || game.window.size.y != windowSizePrepoll.y) {
                log("Processing window resize...");

                glViewport(0, 0, game.window.size.x, game.window.size.y);

                if (!game.renderer.resize_surfaces(game.window.size)) {
                    return false;
                }
            }
        }

        return true;
    }

    bool run_game(const GameInfoInitializer gameInfoInitializer) {
        //
        // Loading User Game Information
        //

        // Initialise the game information struct with some default game settings.
        GameInfo gameInfo = {
            .permMemArenaSize = megabytes_to_bytes(64),
            .tempMemArenaSize = megabytes_to_bytes(4),

            .windowInitSize = {1280, 720}, // NOTE: Could use a function pointer here instead, to allow for the size to be dynamically determined based on display size for example.
            .windowTitle = "ZF4 Game",

            .renderBatchLimit = 64,
            .renderBatchLife = 1000,

            .entLimit = 1024
        };

        // Run the initialiser function, which could overwrite some of the above defaults.
        gameInfoInitializer(gameInfo);

        // Verify that things have been set correctly.
        // NOTE: We might need some more checks here.
        assert(gameInfo.init);
        assert(gameInfo.tick);
        assert(gameInfo.draw);
        assert(gameInfo.cleanup);

        if (gameInfo.spriteCnt > 0) {
            assert(gameInfo.spriteInitializer);
        }

        if (gameInfo.componentTypeCnt > 0) {
            assert(gameInfo.componentTypeInitializer);
        }

        //
        // Running the Game
        //
        Game game = {};
        const bool success = exec_game_init_and_main_loop(game, gameInfo);

        //
        // Cleanup
        //
        gameInfo.cleanup();

        game.renderer.clean();

        clean_music_srcs(&game.musicSrcManager);

        clean_sound_srcs(&game.sndSrcManager);

        game.assets.clean();

        clean_audio_system();

        if (game.window.glfwWindow) {
            glfwDestroyWindow(game.window.glfwWindow);
        }

        glfwTerminate();

        game.tempMemArena.clean();
        game.permMemArena.clean();

        return success;
    }
}
