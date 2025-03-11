package zf4

import "core:c"
import "core:fmt"
import "core:mem"

import gl "vendor:OpenGL"
import "vendor:glfw"

GL_VERS_MAJOR :: 4
GL_VERS_MINOR :: 3

TARG_TICKS_PER_SEC :: 60
TARG_TICK_DUR_SECS :: f64(1.0) / f64(TARG_TICKS_PER_SEC)

// TODO: Update validity checking.
Game_Info :: struct {
	perm_mem_arena_size:     u32, // NOTE: If either arena is given too small a size, the game will fail to initialise.
	temp_mem_arena_size:     u32,
	window_init_size:        Vec_2D_I,
	window_title:            cstring,
	tex_cnt:                 int,
	tex_index_to_file_path:  Texture_Index_To_File_Path,
	font_cnt:                int,
	font_index_to_load_info: Font_Index_To_Load_Info,
	init_func:               proc(func_data: ^Game_Init_Func_Data) -> bool,
	tick_func:               proc(func_data: ^Game_Tick_Func_Data) -> bool,
	draw_func:               proc(func_data: ^Game_Draw_Func_Data) -> bool,
}

Game_Init_Func_Data :: struct {
	input_state:              ^Input_State,
	temp_mem_arena_allocator: mem.Allocator,
}

Game_Tick_Func_Data :: struct {
	input_state:              ^Input_State,
	input_state_last:         ^Input_State,
	temp_mem_arena_allocator: mem.Allocator,
}

Game_Draw_Func_Data :: struct {
	draw_phase_state:         ^Draw_Phase_State,
	pers_render_data:         ^Pers_Render_Data,
	temp_mem_arena_allocator: mem.Allocator,
}

run_game :: proc(info: Game_Info) -> bool {
	//
	// Initialisation
	//
	fmt.println("Initialising...")

	// Set up the main memory arena.
	perm_mem_arena_buf := make([]byte, info.perm_mem_arena_size)

	if (perm_mem_arena_buf == nil) {
		fmt.eprintln("Failed to allocate memory for the main memory arena!")
		return false
	}

	defer delete(perm_mem_arena_buf)

	perm_mem_arena: mem.Arena
	mem.arena_init(&perm_mem_arena, perm_mem_arena_buf)

	perm_mem_arena_allocator := mem.arena_allocator(&perm_mem_arena)

	// Set up the temporary memory arena.
	temp_mem_arena_buf := make([]byte, info.temp_mem_arena_size)

	if (temp_mem_arena_buf == nil) {
		fmt.eprintln("Failed to allocate memory for the temporary memory arena!")
		return false
	}

	defer delete(temp_mem_arena_buf)

	temp_mem_arena: mem.Arena
	mem.arena_init(&temp_mem_arena, temp_mem_arena_buf)

	temp_mem_arena_allocator := mem.arena_allocator(&temp_mem_arena)

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
		nil,
	)

	if glfw_window == nil {
		return false
	}

	defer glfw.DestroyWindow(glfw_window)

	glfw.MakeContextCurrent(glfw_window)

	glfw.SwapInterval(1)

	//
	gl.load_up_to(GL_VERS_MAJOR, GL_VERS_MINOR, glfw.gl_set_proc_address)

	gl.Enable(gl.BLEND)
	gl.BlendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA)

	pers_render_data, pers_render_data_gen_success := gen_pers_render_data(
		perm_mem_arena_allocator,
		temp_mem_arena_allocator,
		info.tex_cnt,
		info.tex_index_to_file_path,
		info.font_cnt,
		info.font_index_to_load_info,
	)

	if !pers_render_data_gen_success {
		return false
	}

	defer clean_pers_render_data(&pers_render_data)

	//
	input_state := load_input_state(glfw_window)

	{
		func_data := Game_Init_Func_Data {
			input_state              = &input_state,
			temp_mem_arena_allocator = temp_mem_arena_allocator,
		}

		if (!info.init_func(&func_data)) {
			return false
		}
	}

	glfw.ShowWindow(glfw_window)

	//
	// Main Loop
	//
	frame_time := glfw.GetTime()
	frame_dur_accum := TARG_TICK_DUR_SECS // Make sure that we begin with a tick.

	draw_phase_state := new(Draw_Phase_State, perm_mem_arena_allocator)

	if draw_phase_state == nil {
		fmt.eprintf("Failed to allocate memory for draw phase state data!")
		return false
	}

	fmt.println("Entering the main loop...")

	for (!glfw.WindowShouldClose(glfw_window)) {
		input_state_last := input_state
		input_state = load_input_state(glfw_window)

		frame_time_last := frame_time
		frame_time = glfw.GetTime()

		frame_dur := frame_time - frame_time_last
		frame_dur_accum += frame_dur

		if frame_dur_accum >= TARG_TICK_DUR_SECS {
			for frame_dur_accum >= TARG_TICK_DUR_SECS {
				mem.arena_free_all(&temp_mem_arena)

				func_data := Game_Tick_Func_Data {
					input_state              = &input_state,
					input_state_last         = &input_state_last,
					temp_mem_arena_allocator = temp_mem_arena_allocator,
				}

				if !info.tick_func(&func_data) {
					return false
				}

				frame_dur_accum -= TARG_TICK_DUR_SECS
			}

			mem.arena_free_all(&temp_mem_arena)

			begin_draw_phase(draw_phase_state)

			{
				func_data := Game_Draw_Func_Data {
					draw_phase_state         = draw_phase_state,
					pers_render_data         = &pers_render_data,
					temp_mem_arena_allocator = temp_mem_arena_allocator,
				}

				if !info.draw_func(&func_data) {
					return false
				}
			}

			assert(draw_phase_state.batch_slots_used_cnt == 0) // Make sure that we flushed.

			glfw.SwapBuffers(glfw_window)
		}

		glfw.PollEvents()
	}

	return true
}

