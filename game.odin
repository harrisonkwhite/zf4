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
	perm_mem_arena_size:          int, // NOTE: If either arena is given too small a size, the game will fail to initialise.
	temp_mem_arena_size:          int,
	user_mem_size:                int,
	user_mem_alignment:           int,
	window_init_size:             Vec_2D_I,
	window_min_size:              Vec_2D_I,
	window_title:                 cstring,
	tex_cnt:                      int,
	tex_index_to_file_path_func:  Texture_Index_To_File_Path,
	font_cnt:                     int,
	font_index_to_load_info_func: Font_Index_To_Load_Info,
	init_func:                    proc(func_data: ^Game_Init_Func_Data) -> bool,
	tick_func:                    proc(func_data: ^Game_Tick_Func_Data) -> bool,
	draw_func:                    proc(func_data: ^Game_Render_Func_Data) -> bool,
	clean_func:                   proc(user_mem: rawptr),
}

Game_Init_Func_Data :: struct {
	user_mem:           rawptr,
	window_state_cache: Window_State,
	input_state:        ^Input_State,
	perm_allocator:     mem.Allocator,
	scratch_allocator:  mem.Allocator,
}

Game_Tick_Func_Data :: struct {
	user_mem:               rawptr,
	window_state_cache:     Window_State,
	fullscreen_state_ideal: ^bool,
	input_state:            ^Input_State,
	input_state_last:       ^Input_State,
	textures:               ^Textures,
	fonts:                  ^Fonts,
	perm_allocator:         mem.Allocator,
	scratch_allocator:      mem.Allocator,
	exit_game:              ^bool,
}

Game_Render_Func_Data :: struct {
	user_mem:          rawptr,
	rendering_context: Rendering_Context,
	textures:          ^Textures,
	fonts:             ^Fonts,
	scratch_allocator: mem.Allocator,
}

Window_State :: struct {
	pos:        Vec_2D_I,
	size:       Vec_2D_I,
	fullscreen: bool,
}

run_game :: proc(info: Game_Info) -> bool {
	// TODO: Assert correctness of game information data.

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

	if info.window_min_size != {} {
		glfw.SetWindowSizeLimits(
			glfw_window,
			i32(info.window_min_size.x),
			i32(info.window_min_size.y),
			glfw.DONT_CARE,
			glfw.DONT_CARE,
		)
	}

	//
	gl.load_up_to(GL_VERS_MAJOR, GL_VERS_MINOR, glfw.gl_set_proc_address)

	gl.Enable(gl.BLEND)
	gl.BlendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA)

	pers_render_data := gen_pers_render_data()
	defer clean_pers_render_data(&pers_render_data)

	textures, textures_loaded := load_textures(
		perm_mem_arena_allocator,
		info.tex_cnt,
		info.tex_index_to_file_path_func,
	)

	defer unload_textures(&textures)

	if !textures_loaded {
		return false
	}

	fonts, fonts_loaded := load_fonts(
		perm_mem_arena_allocator,
		temp_mem_arena_allocator,
		info.font_cnt,
		info.font_index_to_load_info_func,
	)

	defer unload_fonts(&fonts)

	if !fonts_loaded {
		return false
	}

	defer clean_pers_render_data(&pers_render_data)

	user_mem, user_mem_alloc_err := mem.alloc(
		info.user_mem_size,
		info.user_mem_alignment,
		perm_mem_arena_allocator,
	)

	if user_mem == nil {
		return false
	}

	defer info.clean_func(user_mem)

	defer mem.free(user_mem, perm_mem_arena_allocator)

	//
	input_state := load_input_state(glfw_window)

	{
		func_data := Game_Init_Func_Data {
			user_mem           = user_mem,
			window_state_cache = load_window_state(glfw_window),
			scratch_allocator  = temp_mem_arena_allocator,
			input_state        = &input_state,
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

	rendering_state := new(Rendering_State, perm_mem_arena_allocator)

	if rendering_state == nil {
		fmt.eprintf("Failed to allocate memory for render state data!")
		return false
	}

	window_pos_fullscreen_switch_cache: Vec_2D_I
	window_size_fullscreen_switch_cache: Vec_2D_I

	fmt.println("Entering the main loop...")

	for (!glfw.WindowShouldClose(glfw_window)) {
		window_state_cache := load_window_state(glfw_window)
		fullscreen_state_ideal := window_state_cache.fullscreen

		frame_time_last := frame_time
		frame_time = glfw.GetTime()

		frame_dur := frame_time - frame_time_last
		frame_dur_accum += frame_dur

		if frame_dur_accum >= TARG_TICK_DUR_SECS {
			input_state_last := input_state // PROBLEM
			input_state = load_input_state(glfw_window)

			for frame_dur_accum >= TARG_TICK_DUR_SECS {
				mem.arena_free_all(&temp_mem_arena)

				exit_game: bool

				func_data := Game_Tick_Func_Data {
					user_mem               = user_mem,
					window_state_cache     = window_state_cache,
					fullscreen_state_ideal = &fullscreen_state_ideal,
					input_state            = &input_state,
					input_state_last       = &input_state_last,
					textures               = &textures,
					fonts                  = &fonts,
					scratch_allocator      = temp_mem_arena_allocator,
					exit_game              = &exit_game,
				}

				if !info.tick_func(&func_data) {
					return false
				}

				if exit_game {
					return true
				}

				frame_dur_accum -= TARG_TICK_DUR_SECS
			}

			mem.arena_free_all(&temp_mem_arena)

			begin_rendering(rendering_state)

			{
				func_data := Game_Render_Func_Data {
					user_mem          = user_mem,
					textures          = &textures,
					fonts             = &fonts,
					scratch_allocator = temp_mem_arena_allocator,
				}

				func_data.rendering_context = {
					pers         = &pers_render_data,
					state        = rendering_state,
					display_size = window_state_cache.size,
				}

				if !info.draw_func(&func_data) {
					return false
				}
			}

			assert(rendering_state.batch_slots_used_cnt == 0) // Make sure that we flushed.

			glfw.SwapBuffers(glfw_window)
		}

		// BUG: Maximise on a secondary monitor and go fullscreen; every time you click the window disappears.

		if fullscreen_state_ideal && !window_state_cache.fullscreen {
			monitor := glfw.GetPrimaryMonitor()
			vid_mode := glfw.GetVideoMode(monitor)

			glfw.SetWindowMonitor(
				glfw_window,
				monitor,
				0,
				0,
				vid_mode.width,
				vid_mode.height,
				vid_mode.refresh_rate,
			)

			fmt.println("Going fullscreen...")
		} else if !fullscreen_state_ideal && window_state_cache.fullscreen {
			glfw.SetWindowMonitor(
				glfw_window,
				nil,
				i32(window_pos_fullscreen_switch_cache.x),
				i32(window_pos_fullscreen_switch_cache.y),
				i32(window_size_fullscreen_switch_cache.x),
				i32(window_size_fullscreen_switch_cache.y),
				0,
			)

			fmt.println("Going windowed...")
		}

		glfw.PollEvents()

		// Handle any window state changes.
		window_state_after_poll_events := load_window_state(glfw_window)

		if window_state_after_poll_events.size != {} &&
		   window_state_after_poll_events.size != window_state_cache.size {
			gl.Viewport(
				0,
				0,
				i32(window_state_after_poll_events.size.x),
				i32(window_state_after_poll_events.size.y),
			)
		}

		if window_state_after_poll_events.fullscreen && !window_state_cache.fullscreen {
			window_pos_fullscreen_switch_cache = window_state_cache.pos
			window_size_fullscreen_switch_cache = window_state_cache.size
		}
	}

	return true
}

load_window_state :: proc(glfw_window: glfw.WindowHandle) -> Window_State {
	assert(glfw_window != nil)

	x, y := glfw.GetWindowPos(glfw_window)
	width, height := glfw.GetWindowSize(glfw_window)

	return {
		pos = {int(x), int(y)},
		size = {int(width), int(height)},
		fullscreen = glfw.GetWindowMonitor(glfw_window) != nil,
	}
}

