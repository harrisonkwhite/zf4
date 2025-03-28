package zf4

import "core:c"
import "core:fmt"
import "core:mem"
import "core:strings"

import gl "vendor:OpenGL"
import "vendor:glfw"

GL_VERS_MAJOR :: 4
GL_VERS_MINOR :: 3

TARG_TICKS_PER_SEC :: 60
TARG_TICK_DUR_SECS :: f64(1.0) / f64(TARG_TICKS_PER_SEC)

// TODO: Update validity checking.
Game_Info :: struct {
	perm_mem_arena_size:                  int,
	user_mem_size:                        int,
	user_mem_alignment:                   int,
	window_init_size:                     Vec_2D_I,
	window_min_size:                      Vec_2D_I,
	window_title:                         string,
	window_flags:                         Window_Flag_Set,
	tex_cnt:                              int,
	tex_index_to_file_path_func:          Texture_Index_To_File_Path,
	font_cnt:                             int,
	font_index_to_load_info_func:         Font_Index_To_Load_Info,
	shader_prog_cnt:                      int,
	shader_prog_index_to_file_paths_func: Shader_Prog_Index_To_File_Paths,
	init_func:                            proc(func_data: ^Game_Init_Func_Data) -> bool,
	tick_func:                            proc(func_data: ^Game_Tick_Func_Data) -> bool,
	draw_func:                            proc(func_data: ^Game_Render_Func_Data) -> bool,
	clean_func:                           proc(user_mem: rawptr),
}

Game_Init_Func_Data :: struct {
	user_mem:           rawptr,
	window_state_cache: Window_State,
	input_state:        ^Input_State,
	perm_allocator:     mem.Allocator,
}

Game_Tick_Func_Data :: struct {
	user_mem:               rawptr,
	window_state_cache:     Window_State,
	fullscreen_state_ideal: ^bool,
	input_state:            ^Input_State,
	input_state_last:       ^Input_State,
	textures:               ^Textures,
	fonts:                  ^Fonts,
	shader_progs:           ^Shader_Progs,
	perm_allocator:         mem.Allocator,
	exit_game:              ^bool,
}

Game_Render_Func_Data :: struct {
	user_mem:          rawptr,
	rendering_context: Rendering_Context,
	textures:          ^Textures,
	fonts:             ^Fonts,
	shader_progs:      ^Shader_Progs,
}

Window_State :: struct {
	pos:        Vec_2D_I,
	size:       Vec_2D_I,
	fullscreen: bool,
}

Window_Flag :: enum {
	Resizable,
	Hide_Cursor,
}

Window_Flag_Set :: bit_set[Window_Flag]

run_game :: proc(info: Game_Info) -> bool {
	// TODO: Assert correctness of game information data.

	//
	// Initialisation
	//
	fmt.println("Initialising...")

	// Set up the permanent memory arena.
	perm_mem_arena_buf := make([]byte, info.perm_mem_arena_size)

	if (perm_mem_arena_buf == nil) {
		fmt.eprintln("Failed to allocate memory for the main memory arena!")
		return false
	}

	defer delete(perm_mem_arena_buf)

	perm_mem_arena: mem.Arena
	mem.arena_init(&perm_mem_arena, perm_mem_arena_buf)

	perm_mem_arena_allocator := mem.arena_allocator(&perm_mem_arena)

	//
	if (!glfw.Init()) {
		return false
	}

	defer glfw.Terminate()

	glfw.WindowHint(glfw.CONTEXT_VERSION_MAJOR, GL_VERS_MAJOR)
	glfw.WindowHint(glfw.CONTEXT_VERSION_MINOR, GL_VERS_MINOR)
	glfw.WindowHint(glfw.OPENGL_PROFILE, glfw.OPENGL_CORE_PROFILE)
	glfw.WindowHint(glfw.VISIBLE, false)

	window_title_c_str, window_title_c_str_alloc_err := strings.clone_to_cstring(
		info.window_title,
		perm_mem_arena_allocator,
	)

	if window_title_c_str_alloc_err != nil {
		return false
	}

	glfw_window := glfw.CreateWindow(
		c.int(info.window_init_size.x),
		c.int(info.window_init_size.y),
		window_title_c_str,
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

	glfw.SetWindowAttrib(
		glfw_window,
		glfw.RESIZABLE,
		Window_Flag.Resizable in info.window_flags ? 1 : 0,
	)
	glfw.SetInputMode(
		glfw_window,
		glfw.CURSOR,
		Window_Flag.Hide_Cursor in info.window_flags ? glfw.CURSOR_HIDDEN : glfw.CURSOR_NORMAL,
	)

	//
	gl.load_up_to(GL_VERS_MAJOR, GL_VERS_MINOR, glfw.gl_set_proc_address)

	gl.Enable(gl.BLEND)
	gl.BlendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA)

	pers_render_data, pers_render_data_generated := gen_pers_render_data(info.window_init_size)
	defer clean_pers_render_data(&pers_render_data)

	if !pers_render_data_generated {
		return false
	}

	textures, textures_loaded := load_textures(
		info.tex_cnt,
		info.tex_index_to_file_path_func,
		perm_mem_arena_allocator,
	)

	defer unload_textures(&textures)

	if !textures_loaded {
		return false
	}

	fonts, fonts_loaded := load_fonts(
		perm_mem_arena_allocator,
		info.font_cnt,
		info.font_index_to_load_info_func,
	)

	defer unload_fonts(&fonts)

	if !fonts_loaded {
		return false
	}

	shader_progs, shader_progs_loaded := load_shader_progs(
		perm_mem_arena_allocator,
		info.shader_prog_cnt,
		info.shader_prog_index_to_file_paths_func,
	)

	defer unload_shader_progs(&shader_progs)

	if !shader_progs_loaded {
		return false
	}

	user_mem, user_mem_alloc_err := mem.alloc(
		info.user_mem_size,
		info.user_mem_alignment,
		perm_mem_arena_allocator,
	)

	if user_mem == nil {
		return false
	}

	defer mem.free(user_mem, perm_mem_arena_allocator)

	defer info.clean_func(user_mem)

	//
	input_state := load_input_state(glfw_window)

	{
		func_data := Game_Init_Func_Data {
			user_mem           = user_mem,
			window_state_cache = load_window_state(glfw_window),
			input_state        = &input_state,
		}

		if (!info.init_func(&func_data)) {
			return false
		}
	}

	glfw.ShowWindow(glfw_window)

	free_all(context.temp_allocator)

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
				exit_game: bool

				func_data := Game_Tick_Func_Data {
					user_mem               = user_mem,
					window_state_cache     = window_state_cache,
					fullscreen_state_ideal = &fullscreen_state_ideal,
					input_state            = &input_state,
					input_state_last       = &input_state_last,
					textures               = &textures,
					fonts                  = &fonts,
					shader_progs           = &shader_progs,
					exit_game              = &exit_game,
				}

				if !info.tick_func(&func_data) {
					return false
				}

				free_all(context.temp_allocator)

				if exit_game {
					return true
				}

				frame_dur_accum -= TARG_TICK_DUR_SECS
			}

			begin_rendering(rendering_state)

			{
				func_data := Game_Render_Func_Data {
					user_mem     = user_mem,
					textures     = &textures,
					fonts        = &fonts,
					shader_progs = &shader_progs,
				}

				func_data.rendering_context = {
					pers         = &pers_render_data,
					state        = rendering_state,
					display_size = window_state_cache.size,
				}

				if !info.draw_func(&func_data) {
					return false
				}

				free_all(context.temp_allocator)
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

			if !resize_surfaces(&pers_render_data.surfs, window_state_cache.size) {
				return false
			}
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

