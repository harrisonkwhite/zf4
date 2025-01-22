#include <zf4_game.h>

#include <zf4c_mem.h>
#include <zf4_rand.h>

#define TARG_TICKS_PER_SEC 60
#define TARG_TICK_DUR_SECS (1.0 / TARG_TICKS_PER_SEC)

typedef struct game {
    s_mem_arena perm_mem_arena;
    s_mem_arena temp_mem_arena;
    s_window window;
    s_assets* assets;
    s_renderer* renderer;
} s_game;

static bool ExecGameInitAndMainLoop(s_game* const game, const s_game_info* const game_info) {
    assert(IsClear(game, sizeof(*game)));

    //
    // Initialisation
    //
    Log("Initialising...");

    // Set up memory arenas.
    if (!InitMemArena(&game->perm_mem_arena, game_info->perm_mem_arena_size)) {
        LogError("Failed to initialise the permanent memory arena!");
        return false;
    }

    if (!InitMemArena(&game->temp_mem_arena, game_info->temp_mem_arena_size)) {
        LogError("Failed to initialise the temporary memory arena!");
        return false;
    }

    // Set up GLFW and the window.
    if (!glfwInit()) {
        LogError("Failed to initialise GLFW!");
        return false;
    }

    if (!InitWindow(&game->window, game_info->window_init_size, game_info->window_title, game_info->window_flags)) {
        return false;
    }

    // Set up OpenGL.
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        LogError("Failed to initialise OpenGL function pointers!");
        return false;
    }

    // Load assets.
    game->assets = LoadAssets(&game->perm_mem_arena, &game->temp_mem_arena);

    if (!game->assets) {
        LogError("Failed to load assets!");
        return false;
    }

    // Load the renderer.
    game->renderer = LoadRenderer(&game->perm_mem_arena, &game->temp_mem_arena);

    if (!game->renderer) {
        LogError("Failed to load renderer!");
        return false;
    }

    // Set up the random number generator.
    InitRNG();

    // Define a struct of pointers to game data to pass to the user-defined game functions.
    const s_game_ptrs game_ptrs = {
        .perm_mem_arena = &game->perm_mem_arena,
        .temp_mem_arena = &game->temp_mem_arena,
        .window = &game->window,
        .assets = game->assets,
        .renderer = game->renderer
    };

    // Call the user-defined initialisation function.
    if (!game_info->init_func(&game_ptrs)) {
        return false;
    }

    // Now that everything is initialised, show the window.
    glfwShowWindow(game->window.glfw_window);

    //
    // Main Loop
    //
    double frame_time = glfwGetTime();
    double frame_dur_accum = TARG_TICK_DUR_SECS; // Make sure that we begin with a tick.

    Log("Entering the main loop...");

    while (!glfwWindowShouldClose(game->window.glfw_window)) {
        ResetMemArena(&game->temp_mem_arena);

        const double frame_time_last = frame_time;
        frame_time = glfwGetTime();

        const double frame_dur = frame_time - frame_time_last;
        frame_dur_accum += frame_dur;

        if (frame_dur_accum >= TARG_TICK_DUR_SECS) {
            const double fps = 1.0 / frame_dur;

            do {
                // Execute a tick.
                if (!game_info->tick_func(&game_ptrs, fps)) {
                    return false;
                }

                frame_dur_accum -= TARG_TICK_DUR_SECS;
            } while (frame_dur_accum >= TARG_TICK_DUR_SECS);

            game->window.input_state_saved = game->window.input_state;

            // Execute draw.
            s_draw_phase_state* const draw_phase_state = BeginDrawPhase(&game->temp_mem_arena, game->window.size_cache, game->assets);

            if (!game_info->draw_func(draw_phase_state, &game_ptrs, fps)) {
                return false;
            }

            assert(draw_phase_state->tex_batch_slots_used_cnt == 0); // Make sure that the last batch was flushed.

            glfwSwapBuffers(game->window.glfw_window);
        }

        const s_vec_2d_i window_size_prepoll = game->window.size_cache;

        glfwPollEvents();

        // Check for and process window resize.
        if (game->window.size_cache.x != window_size_prepoll.x || game->window.size_cache.y != window_size_prepoll.y) {
            Log("Processing window resize...");

            glViewport(0, 0, game->window.size_cache.x, game->window.size_cache.y);

            if (!ResizeRenderSurfaces(&game->renderer->surfs, game->window.size_cache)) {
                return false;
            }
        }
    }

    return true;
}

bool RunGame(const ta_game_info_loader info_loader) {
    assert(info_loader);

    //
    // Loading Game Information
    //
    s_game_info info = {
        .perm_mem_arena_size = MegabytesToBytes(80),
        .temp_mem_arena_size = MegabytesToBytes(40),

        .window_init_size = {1280, 720},
        .window_title = "ZF4 Game"
    };

    // Run the user-defined function which should overwrite some of the defaults.
    info_loader(&info);

    // Check that the fields are set correctly.
    assert(info.init_func);
    assert(info.tick_func);
    assert(info.draw_func);
    assert(info.cleanup_func);
    assert(info.perm_mem_arena_size > 0);
    assert(info.temp_mem_arena_size > 0);
    assert(info.window_init_size.x > 0 && info.window_init_size.y > 0);
    assert(info.window_title);

    //
    // Running the Game
    //
    s_game game = {0};

    const bool success = ExecGameInitAndMainLoop(&game, &info);

    //
    // Cleanup
    //
    info.cleanup_func();

    if (game.renderer) {
        CleanRenderer(game.renderer);
    }

    if (game.assets) {
        UnloadAssets(game.assets);
    }

    CleanWindow(&game.window);

    glfwTerminate();

    CleanMemArena(&game.temp_mem_arena);
    CleanMemArena(&game.perm_mem_arena);

    return success;
}
