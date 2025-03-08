package zf4

import "core:c"
import "core:fmt"

import gl "vendor:OpenGL"
import "vendor:glfw"

GL_VERS_MAJOR :: 4
GL_VERS_MINOR :: 3

TARG_TICKS_PER_SEC :: 60
TARG_TICK_DUR_SECS :: 1.0 / TARG_TICKS_PER_SEC

Game_Info :: struct {
	perm_mem_arena_size: u32,
	temp_mem_arena_size: u32,

	window_init_size:    Size_2D,
	window_title:        cstring,

    init_func:           proc(zf4_data: Game_Init_Func_Data) -> bool,
    tick_func:           proc(zf4_data: Game_Tick_Func_Data) -> bool,
}

Game_Init_Func_Data :: struct {
    input_state: Input_State,
}

Game_Tick_Func_Data :: struct {
    input_state: Input_State,
    input_state_last: Input_State
}

run_game :: proc(info: Game_Info) -> bool {
	assert(is_game_info_valid(info))

    //
    // Initialisation
    //
	if (!glfw.Init()) {
		return false
	}

	defer glfw.Terminate()

	glfw.WindowHint(glfw.CONTEXT_VERSION_MAJOR, GL_VERS_MAJOR)
	glfw.WindowHint(glfw.CONTEXT_VERSION_MINOR, GL_VERS_MINOR)
	glfw.WindowHint(glfw.OPENGL_PROFILE, glfw.OPENGL_CORE_PROFILE)
    glfw.WindowHint(glfw.VISIBLE, false)

	glfw_window := glfw.CreateWindow(
		c.int(info.window_init_size.x),
		c.int(info.window_init_size.y),
		info.window_title,
		nil,
		nil
	)

	if (glfw_window == nil) {
		return false
	}

	defer glfw.DestroyWindow(glfw_window)

	glfw.MakeContextCurrent(glfw_window)

	glfw.SwapInterval(1)

	gl.load_up_to(GL_VERS_MAJOR, GL_VERS_MINOR, glfw.gl_set_proc_address)

    input_state := load_input_state(glfw_window)

    {
        func_data := Game_Init_Func_Data {
            input_state = input_state,
        }

        if (!info.init_func(func_data)) {
            return false
        }
    }

    glfw.ShowWindow(glfw_window)

    //
    // Main Loop
    //
    frame_time := glfw.GetTime()
    frame_dur_accum := TARG_TICK_DUR_SECS // Make sure that we begin with a tick.

    fmt.println("Entering the main loop...")

	for (!glfw.WindowShouldClose(glfw_window)) {
		input_state_last := input_state
        input_state = load_input_state(glfw_window)

        frame_time_last := frame_time;
        frame_time = glfw.GetTime();

        frame_dur := frame_time - frame_time_last;
        frame_dur_accum += frame_dur;

        if frame_dur_accum >= TARG_TICK_DUR_SECS {
            for frame_dur_accum >= TARG_TICK_DUR_SECS {
                func_data := Game_Tick_Func_Data {
                    input_state = input_state,
                    input_state_last = input_state_last
                }

                if (!info.tick_func(func_data)) {
                    return false
                }

                frame_dur_accum -= TARG_TICK_DUR_SECS
            }

            gl.ClearColor(0.2, 0.3, 0.3, 1.0)
            gl.Clear(gl.COLOR_BUFFER_BIT)

            glfw.SwapBuffers(glfw_window)
        }

		glfw.PollEvents()
	}

	return true
}

is_game_info_valid :: proc(info: Game_Info) -> bool {
	return(
		info.perm_mem_arena_size > 0 &&
		info.temp_mem_arena_size > 0 &&
        info.window_init_size.x > 0 &&
		info.window_init_size.y > 0 \
	)
}
