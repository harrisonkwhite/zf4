#include <zf4_game.h>

#include <zf4c_mem.h>
#include <zf4_rand.h>

namespace zf4 {
    constexpr int g_targ_ticks_per_sec = 60;
    constexpr double g_targ_tick_dur_secs = 1.0 / g_targ_ticks_per_sec;

    struct s_game {
        s_mem_arena perm_mem_arena;
        s_mem_arena temp_mem_arena;
        s_window window;
        s_assets* assets;
        s_pers_render_data* pers_render_data;
        void* custom_data;
    };

    static bool ExecGameInitAndMainLoop(s_game& game, const s_game_info& game_info) {
        assert(IsStructZero(game));

        //
        // Initialisation
        //
        Log("Initialising...");

        // Set up memory arenas.
        if (!InitMemArena(game.perm_mem_arena, game_info.perm_mem_arena_size)) {
            LogError("Failed to initialise the permanent memory arena!");
            return false;
        }

        if (!InitMemArena(game.temp_mem_arena, game_info.temp_mem_arena_size)) {
            LogError("Failed to initialise the temporary memory arena!");
            return false;
        }

        // Set up GLFW and the window.
        if (!glfwInit()) {
            LogError("Failed to initialise GLFW!");
            return false;
        }

        if (!InitWindow(game.window, game_info.window_init_size, game_info.window_title, game_info.window_flags)) {
            return false;
        }

        // Set up OpenGL.
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            LogError("Failed to initialise OpenGL function pointers!");
            return false;
        }

        // Load assets.
        game.assets = LoadAssets(game.perm_mem_arena, game.temp_mem_arena);

        if (!game.assets) {
            LogError("Failed to load assets!");
            return false;
        }

        // Load persistent render data.
        game.pers_render_data = LoadPersRenderData(game.perm_mem_arena, game.temp_mem_arena);

        if (!game.pers_render_data) {
            LogError("Failed to load persistent render data!");
            return false;
        }

        // Set up the random number generator.
        InitRNG();

        // Reserve memory in the permanent arena for user data.
        if (game_info.custom_data_size > 0) {
            game.custom_data = Push(game_info.custom_data_size, game_info.custom_data_alignment, game.perm_mem_arena);

            if (!game.custom_data) {
                LogError("Failed to reserve memory for user data!");
                return false;
            }
        }

        // Define a struct of pointers to game data to pass to the user-defined game functions.
        const s_game_ptrs game_ptrs = {
            .perm_mem_arena = game.perm_mem_arena,
            .temp_mem_arena = game.temp_mem_arena,
            .window = game.window,
            .assets = *game.assets,
            .pers_render_data = *game.pers_render_data,
            .custom_data = game.custom_data
        };

        // Call the user-defined initialisation function.
        if (!game_info.init_func(game_ptrs)) {
            return false;
        }

        // Now that everything is initialised, show the window.
        glfwShowWindow(game.window.glfw_window);

        //
        // Main Loop
        //
        double frame_time = glfwGetTime();
        double frame_dur_accum = g_targ_tick_dur_secs; // Make sure that we begin with a tick.

        Log("Entering the main loop...");

        while (!glfwWindowShouldClose(game.window.glfw_window)) {
            EmptyMemArena(game.temp_mem_arena);

            const double frame_time_last = frame_time;
            frame_time = glfwGetTime();

            const double frame_dur = frame_time - frame_time_last;
            frame_dur_accum += frame_dur;

            if (frame_dur_accum >= g_targ_tick_dur_secs) {
                const double fps = 1.0 / frame_dur;

                do {
                    // Execute a tick.
                    const e_game_tick_func_result tick_result = game_info.tick_func(game_ptrs, fps);

                    if (tick_result != ek_game_tick_func_result_continue) {
                        return tick_result == ek_game_tick_func_result_success_quit;
                    }

                    frame_dur_accum -= g_targ_tick_dur_secs;
                } while (frame_dur_accum >= g_targ_tick_dur_secs);

                game.window.input_state_saved = game.window.input_state;

                // Execute draw.
                s_draw_phase_state* const draw_phase_state = BeginDrawPhase(game.temp_mem_arena, game.window.size_cache, *game.assets);

                if (!draw_phase_state) {
                    return false;
                }

                if (!game_info.draw_func(*draw_phase_state, game_ptrs, fps)) {
                    return false;
                }

                assert(draw_phase_state->tex_batch_slots_used_cnt == 0); // Make sure the last batch was flushed.

                glfwSwapBuffers(game.window.glfw_window);
            }

            const s_vec_2d_i window_size_prepoll = game.window.size_cache;

            glfwPollEvents();

            // Check for and process window resize.
            if (game.window.size_cache.x != window_size_prepoll.x || game.window.size_cache.y != window_size_prepoll.y) {
                Log("Processing window resize...");

                glViewport(0, 0, game.window.size_cache.x, game.window.size_cache.y);

                if (!ResizeRenderSurfaces(game.pers_render_data->surfs, game.window.size_cache)) {
                    return false;
                }
            }
        }

        return true;
    }

    bool RunGame(const a_game_info_loader info_loader) {
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
        info_loader(info);

        // Check that the fields are set correctly.
        assert(info.init_func);
        assert(info.tick_func);
        assert(info.draw_func);
        assert(info.perm_mem_arena_size > 0);
        assert(info.temp_mem_arena_size > 0);
        assert(info.window_init_size.x > 0 && info.window_init_size.y > 0);
        assert(info.window_title);
        assert(info.custom_data_size >= 0);
        assert(info.custom_data_size > 0 ? info.custom_data_alignment >= 0 : info.custom_data_alignment == 0);

        //
        // Running the Game
        //
        s_game game = {};

        const bool success = ExecGameInitAndMainLoop(game, info);

        //
        // Cleanup
        //
        if (game.pers_render_data) {
            CleanPersRenderData(*game.pers_render_data);
        }

        if (game.assets) {
            UnloadAssets(*game.assets);
        }

        CleanWindow(game.window);

        glfwTerminate();

        CleanMemArena(game.temp_mem_arena);
        CleanMemArena(game.perm_mem_arena);

        return success;
    }
}
