#include <zf4_game.h>

#include <GLFW/glfw3.h>
#include <zf4_rand.h>
#include <zf4_io.h>

namespace zf4 {
    static constexpr int g_targ_ticks_per_sec = 60;
    static constexpr double g_targ_tick_dur_secs = 1.0 / g_targ_ticks_per_sec;

    struct s_game_cleanup_info {
        s_mem_arena* perm_mem_arena;
        s_mem_arena* temp_mem_arena;

        bool glfw_initialized;
        GLFWwindow* glfw_window;

        const graphics::s_textures* textures;
        const graphics::s_fonts* fonts;

        const graphics::s_pers_render_data* pers_render_data;
        graphics::s_surfaces* surfs;
    };

    static int ToGLFWKeyCode(const e_key_code key_code) {
        switch (key_code) {
            case ek_key_code_space: return GLFW_KEY_SPACE;

            case ek_key_code_0: return GLFW_KEY_0;
            case ek_key_code_1: return GLFW_KEY_1;
            case ek_key_code_2: return GLFW_KEY_2;
            case ek_key_code_3: return GLFW_KEY_3;
            case ek_key_code_4: return GLFW_KEY_4;
            case ek_key_code_5: return GLFW_KEY_5;
            case ek_key_code_6: return GLFW_KEY_6;
            case ek_key_code_7: return GLFW_KEY_7;
            case ek_key_code_8: return GLFW_KEY_8;
            case ek_key_code_9: return GLFW_KEY_9;

            case ek_key_code_a: return GLFW_KEY_A;
            case ek_key_code_b: return GLFW_KEY_B;
            case ek_key_code_c: return GLFW_KEY_C;
            case ek_key_code_d: return GLFW_KEY_D;
            case ek_key_code_e: return GLFW_KEY_E;
            case ek_key_code_f: return GLFW_KEY_F;
            case ek_key_code_g: return GLFW_KEY_G;
            case ek_key_code_h: return GLFW_KEY_H;
            case ek_key_code_i: return GLFW_KEY_I;
            case ek_key_code_j: return GLFW_KEY_J;
            case ek_key_code_k: return GLFW_KEY_K;
            case ek_key_code_l: return GLFW_KEY_L;
            case ek_key_code_m: return GLFW_KEY_M;
            case ek_key_code_n: return GLFW_KEY_N;
            case ek_key_code_o: return GLFW_KEY_O;
            case ek_key_code_p: return GLFW_KEY_P;
            case ek_key_code_q: return GLFW_KEY_Q;
            case ek_key_code_r: return GLFW_KEY_R;
            case ek_key_code_s: return GLFW_KEY_S;
            case ek_key_code_t: return GLFW_KEY_T;
            case ek_key_code_u: return GLFW_KEY_U;
            case ek_key_code_v: return GLFW_KEY_V;
            case ek_key_code_w: return GLFW_KEY_W;
            case ek_key_code_x: return GLFW_KEY_X;
            case ek_key_code_y: return GLFW_KEY_Y;
            case ek_key_code_z: return GLFW_KEY_Z;

            case ek_key_code_escape: return GLFW_KEY_ESCAPE;
            case ek_key_code_enter: return GLFW_KEY_ENTER;
            case ek_key_code_tab: return GLFW_KEY_TAB;

            case ek_key_code_right: return GLFW_KEY_RIGHT;
            case ek_key_code_left: return GLFW_KEY_LEFT;
            case ek_key_code_down: return GLFW_KEY_DOWN;
            case ek_key_code_up: return GLFW_KEY_UP;

            case ek_key_code_f1: return GLFW_KEY_F1;
            case ek_key_code_f2: return GLFW_KEY_F2;
            case ek_key_code_f3: return GLFW_KEY_F3;
            case ek_key_code_f4: return GLFW_KEY_F4;
            case ek_key_code_f5: return GLFW_KEY_F5;
            case ek_key_code_f6: return GLFW_KEY_F6;
            case ek_key_code_f7: return GLFW_KEY_F7;
            case ek_key_code_f8: return GLFW_KEY_F8;
            case ek_key_code_f9: return GLFW_KEY_F9;
            case ek_key_code_f10: return GLFW_KEY_F10;
            case ek_key_code_f11: return GLFW_KEY_F11;
            case ek_key_code_f12: return GLFW_KEY_F12;

            case ek_key_code_left_shift: return GLFW_KEY_LEFT_SHIFT;
            case ek_key_code_left_control: return GLFW_KEY_LEFT_CONTROL;
            case ek_key_code_left_alt: return GLFW_KEY_LEFT_ALT;
            case ek_key_code_right_shift: return GLFW_KEY_RIGHT_SHIFT;
            case ek_key_code_right_control: return GLFW_KEY_RIGHT_CONTROL;
            case ek_key_code_right_alt: return GLFW_KEY_RIGHT_ALT;

            default: return -1;
        }
    }

    static int ToGLFWMouseButtonCode(const e_mouse_button_code button_code) {
        switch (button_code) {
            case ek_mouse_button_code_left: return GLFW_MOUSE_BUTTON_LEFT;
            case ek_mouse_button_code_right: return GLFW_MOUSE_BUTTON_RIGHT;
            case ek_mouse_button_code_middle: return GLFW_MOUSE_BUTTON_MIDDLE;

            default: return -1;
        }
    }

    static inline s_vec_2d_i WindowSize(GLFWwindow* const glfw_window) {
        s_vec_2d_i size;
        glfwGetWindowSize(glfw_window, &size.x, &size.y);
        return size;
    }

    static s_window_state LoadWindowState(const s_vec_2d_i fallback_size, GLFWwindow* const glfw_window) {
        s_vec_2d_i size;
        glfwGetWindowSize(glfw_window, &size.x, &size.y);

        return {
            .size = IsStructZero(size) ? fallback_size : size,
            .fullscreen = glfwGetWindowMonitor(glfw_window) != nullptr
        };
    }

    static a_keys_down_bits LoadKeysDown(GLFWwindow* const glfw_window) {
        a_keys_down_bits bits = 0;

        for (int i = 0; i < eks_key_code_cnt; ++i) {
            if (glfwGetKey(glfw_window, ToGLFWKeyCode(static_cast<e_key_code>(i))) == GLFW_PRESS) {
                bits |= static_cast<a_keys_down_bits>(1) << i;
            }
        }

        return bits;
    }

    static a_mouse_buttons_down_bits LoadMouseButtonsDown(GLFWwindow* const glfw_window) {
        a_mouse_buttons_down_bits bits = 0;

        for (int i = 0; i < eks_mouse_button_code_cnt; ++i) {
            if (glfwGetMouseButton(glfw_window, ToGLFWMouseButtonCode(static_cast<e_mouse_button_code>(i))) == GLFW_PRESS) {
                bits |= static_cast<a_mouse_buttons_down_bits>(1) << i;
            }
        }

        return bits;
    }

    static s_vec_2d MousePos(GLFWwindow* const glfw_window) {
        double mouse_x_dbl, mouse_y_dbl;
        glfwGetCursorPos(glfw_window, &mouse_x_dbl, &mouse_y_dbl);
        return {static_cast<float>(mouse_x_dbl), static_cast<float>(mouse_y_dbl)};
    }

    static s_input_state LoadInputState(GLFWwindow* const glfw_window) {
        return {
            .keys_down = LoadKeysDown(glfw_window),
            .mouse_buttons_down = LoadMouseButtonsDown(glfw_window),
            .mouse_pos = MousePos(glfw_window)
        };
    }

    static void CleanGame(const s_game_cleanup_info& cleanup_info) {
        if (cleanup_info.surfs) {
            graphics::CleanAndZeroOutSurfaces(*cleanup_info.surfs);
        }

        if (cleanup_info.pers_render_data) {
            graphics::CleanPersRenderData(*cleanup_info.pers_render_data);
        }

        if (cleanup_info.fonts) {
            graphics::UnloadFonts(*cleanup_info.fonts);
        }

        if (cleanup_info.textures) {
            graphics::UnloadTextures(*cleanup_info.textures);
        }

        if (cleanup_info.glfw_window) {
            assert(cleanup_info.glfw_initialized);
            glfwDestroyWindow(cleanup_info.glfw_window);
        }

        if (cleanup_info.glfw_initialized) {
            glfwTerminate();
        }

        if (cleanup_info.temp_mem_arena) {
            CleanMemArena(*cleanup_info.temp_mem_arena);
        }

        if (cleanup_info.perm_mem_arena) {
            CleanMemArena(*cleanup_info.perm_mem_arena);
        }
    }

    bool RunGame(const s_game_info& game_info) {
        // TODO: Verify the game information struct.

        s_game_cleanup_info cleanup_info = {};

        //
        // Initialisation
        //
        bool err = false;

        // Set up memory arenas.
        s_mem_arena perm_mem_arena = GenMemArena(game_info.perm_mem_arena_size, err);

        if (err) {
            LogError("Failed to initialise the permanent memory arena!");
            CleanGame(cleanup_info);
            return false;
        }

        cleanup_info.perm_mem_arena = &perm_mem_arena;

        s_mem_arena temp_mem_arena = GenMemArena(game_info.temp_mem_arena_size, err);

        if (err) {
            LogError("Failed to initialise the temporary memory arena!");
            CleanGame(cleanup_info);
            return false;
        }

        cleanup_info.temp_mem_arena = &temp_mem_arena;

        // Set up GLFW and the window.
        if (!glfwInit()) {
            LogError("Failed to initialize GLFW!");
            CleanGame(cleanup_info);
            return false;
        }

        cleanup_info.glfw_initialized = true;

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, g_gl_version_major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, g_gl_version_minor);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RESIZABLE, game_info.window_flags & ek_window_flags_resizable);
        glfwWindowHint(GLFW_VISIBLE, false);

        GLFWwindow* const glfw_window = glfwCreateWindow(game_info.window_size_default.x, game_info.window_size_default.y, game_info.window_title, nullptr, nullptr);

        if (!glfw_window) {
            LogError("Failed to create a GLFW window!");
            CleanGame(cleanup_info);
            return false;
        }

        cleanup_info.glfw_window = glfw_window;

        glfwMakeContextCurrent(glfw_window);

        glfwSwapInterval(1); // Enables VSync.

        glfwSetWindowSizeLimits(glfw_window, game_info.window_size_min.x, game_info.window_size_min.y, GLFW_DONT_CARE, GLFW_DONT_CARE);

        glfwSetInputMode(glfw_window, GLFW_CURSOR, (game_info.window_flags & ek_window_flags_hide_cursor) ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);

        // Set up OpenGL.
        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
            LogError("Failed to initialise OpenGL function pointers!");
            CleanGame(cleanup_info);
            return false;
        }

        // Load textures.
        const graphics::s_textures textures = graphics::PushTextures(game_info.tex_cnt, game_info.tex_filenames, perm_mem_arena, err);

        if (err) {
            LogError("Failed to load textures!");
            CleanGame(cleanup_info);
            return false;
        }

        cleanup_info.textures = &textures;

        // Load fonts.
        const graphics::s_fonts fonts = graphics::PushFonts(game_info.font_cnt, game_info.font_filenames, game_info.font_pt_sizes, perm_mem_arena, temp_mem_arena, err);

        if (err) {
            LogError("Failed to load fonts!");
            return false;
        }

        cleanup_info.fonts = &fonts;

        // Load persistent render data.
        const graphics::s_pers_render_data pers_render_data = graphics::GenPersRenderData();
        cleanup_info.pers_render_data = &pers_render_data;

        // Zero out surface data; this will only be initialised by the user if they want.
        graphics::s_surfaces surfs = {};

        // Set up the random number generator.
        InitRNG();

        // Reserve memory in the permanent arena for user data.
        void* const custom_data = Push(game_info.custom_data_size, game_info.custom_data_alignment, perm_mem_arena);

        if (!custom_data && game_info.custom_data_size > 0) {
            LogError("Failed to reserve memory for user data!");
            CleanGame(cleanup_info);
            return false;
        }

        // Store the initial window and input states.
        s_window_state window_state_cache = LoadWindowState({}, glfw_window);
        s_input_state input_state = LoadInputState(glfw_window);

        // Call the user-defined initialisation function.
        {
            const s_game_init_func_data func_data = {
                .perm_mem_arena = perm_mem_arena,
                .temp_mem_arena = temp_mem_arena,
                .window_state_cache = window_state_cache,
                .input_state = input_state,
                .textures = textures,
                .fonts = fonts,
                .pers_render_data = pers_render_data,
                .surfs = surfs,
                .custom_data = custom_data
            };

            if (!game_info.init_func(func_data)) {
                CleanGame(cleanup_info);
                return false;
            }
        }

        // Now that everything is initialised, show the window.
        glfwShowWindow(glfw_window);

        //
        // Main Loop
        //
        double frame_time = glfwGetTime();
        double frame_dur_accum = g_targ_tick_dur_secs; // Make sure that we begin with a tick.

        Log("Entering the main loop...");

        while (!glfwWindowShouldClose(glfw_window)) {
            if (!glfwGetWindowAttrib(glfw_window, GLFW_FOCUSED)) {
                glfwPollEvents();
                continue;
            }

            window_state_cache = LoadWindowState(window_state_cache.size, glfw_window);
            s_window_state window_state_ideal = window_state_cache;

            const double frame_time_last = frame_time;
            frame_time = glfwGetTime();

            const double frame_dur = frame_time - frame_time_last;
            frame_dur_accum += frame_dur;

            if (frame_dur_accum >= g_targ_tick_dur_secs) {
                const double fps = 1.0 / frame_dur;

                const s_input_state input_state_last = input_state;
                input_state = LoadInputState(glfw_window);

                do {
                    EmptyMemArena(temp_mem_arena);

                    // Execute a tick.
                    const s_game_tick_func_data func_data = {
                        .perm_mem_arena = perm_mem_arena,
                        .temp_mem_arena = temp_mem_arena,
                        .window_state_cache = window_state_cache,
                        .window_state_ideal = window_state_ideal,
                        .input_state = input_state,
                        .input_state_last = input_state_last,
                        .textures = textures,
                        .fonts = fonts,
                        .pers_render_data = pers_render_data,
                        .surfs = surfs,
                        .fps = fps,
                        .custom_data = custom_data
                    };

                    const e_game_tick_func_result tick_result = game_info.tick_func(func_data);

                    if (tick_result != ek_game_tick_func_result_continue) {
                        return tick_result == ek_game_tick_func_result_success_quit;
                    }

                    frame_dur_accum -= g_targ_tick_dur_secs;
                } while (frame_dur_accum >= g_targ_tick_dur_secs);

                // Execute draw.
                EmptyMemArena(temp_mem_arena);

                {
                    auto instrs = PushList<graphics::s_draw_instr>(game_info.draw_instr_limit, temp_mem_arena);

                    const s_game_draw_func_data func_data = {
                        .temp_mem_arena = temp_mem_arena,
                        .window_state_cache = window_state_cache,
                        .mouse_pos = input_state.mouse_pos,
                        .textures = textures,
                        .fonts = fonts,
                        .pers_render_data = pers_render_data,
                        .fps = fps,
                        .custom_data = custom_data
                    };

                    if (IsStructZero(instrs) || !game_info.append_draw_instrs_func(instrs, func_data)) {
                        LogError("Failed to load draw instructions!");
                        CleanGame(cleanup_info);
                        return false;
                    }

                    if (!graphics::ExecDrawInstrs(ToArray(instrs), window_state_cache.size, pers_render_data, surfs, textures, temp_mem_arena)) {
                        LogError("Failed to execute draw instructions!");
                        CleanGame(cleanup_info);
                        return false;
                    }
                }

                glfwSwapBuffers(glfw_window);
            }

            if (window_state_ideal.fullscreen && !window_state_cache.fullscreen) {
                // Switch from windowed to fullscreen.
                GLFWmonitor* const primary_monitor = glfwGetPrimaryMonitor();
                const GLFWvidmode* video_mode = glfwGetVideoMode(primary_monitor);
                glfwSetWindowMonitor(glfw_window, primary_monitor, 0, 0, video_mode->width, video_mode->height, video_mode->refreshRate);
            } else if (!window_state_ideal.fullscreen && window_state_cache.fullscreen) {
                // Switch from fullscreen to windowed.
                const auto highest_res = []() -> s_vec_2d_i {
                    GLFWmonitor* const primary_monitor = glfwGetPrimaryMonitor();

                    int mode_cnt;
                    const GLFWvidmode* const video_modes = glfwGetVideoModes(primary_monitor, &mode_cnt);

                    const GLFWvidmode* best_mode = &video_modes[0];

                    for (int i = 1; i < mode_cnt; i++) {
                        if (video_modes[i].width > best_mode->width || video_modes[i].height > best_mode->height) {
                            best_mode = &video_modes[i];
                        }
                    }

                    return {best_mode->width, best_mode->height};
                }();

                const s_vec_2d_i central_pos = {
                    (highest_res.x - window_state_ideal.size.x) / 2.0f,
                    (highest_res.y - window_state_ideal.size.y) / 2.0f
                };

                glfwSetWindowMonitor(glfw_window, nullptr, central_pos.x, central_pos.y, window_state_ideal.size.x, window_state_ideal.size.y, GLFW_DONT_CARE);
            } else if (!window_state_cache.fullscreen && window_state_ideal.size != window_state_cache.size) {
                // Update window size if requested and in windowed mode.
                glfwSetWindowSize(glfw_window, window_state_ideal.size.x, window_state_ideal.size.y);
            }

            glfwPollEvents();

            const s_vec_2d_i window_size_after_poll_events = WindowSize(glfw_window);

            if (!IsStructZero(window_size_after_poll_events) && window_size_after_poll_events != window_state_cache.size) {
                glViewport(0, 0, window_size_after_poll_events.x, window_size_after_poll_events.y);

                if (!graphics::ResizeSurfaces(surfs, window_size_after_poll_events)) {
                    CleanGame(cleanup_info);
                    return false;
                }
            }
        }

        CleanGame(cleanup_info);

        return true;
    }
}
