package zf4

import rt "base:runtime"
import "core:fmt"
import "core:mem"
import "core:os"
import gl "vendor:OpenGL"
import stbi "vendor:stb/image"
import stbtt "vendor:stb/truetype"

TEXTURE_CHANNEL_CNT :: 4

FONT_CHR_RANGE_BEGIN :: 32
FONT_CHR_RANGE_LEN :: 95
FONT_TEXTURE_WIDTH :: 2048
FONT_TEXTURE_HEIGHT_LIMIT :: 2048

BATCH_SHADER_PROG_VERT_CNT :: 13
BATCH_SLOT_CNT :: 2048
BATCH_SLOT_VERT_CNT :: BATCH_SHADER_PROG_VERT_CNT * 4
BATCH_SLOT_VERTS_SIZE :: BATCH_SLOT_VERT_CNT * BATCH_SLOT_CNT
BATCH_SLOT_ELEM_CNT :: 6

WHITE :: Vec_4D{1.0, 1.0, 1.0, 1.0}
BLACK :: Vec_4D{0.0, 0.0, 0.0, 1.0}
RED :: Vec_4D{1.0, 0.0, 0.0, 1.0}
GREEN :: Vec_4D{0.0, 1.0, 0.0, 1.0}
BLUE :: Vec_4D{0.0, 0.0, 1.0, 1.0}

Rendering_Context :: struct {
	pers:         ^Pers_Render_Data,
	state:        ^Rendering_State,
	display_size: Vec_2D_I,
}

Pers_Render_Data :: struct {
	batch_shader_prog: Batch_Shader_Prog,
	batch_gl_ids:      Batch_GL_IDs,
}

Rendering_State :: struct {
	batch_slots_used_cnt: int,
	batch_slot_verts:     [BATCH_SLOT_CNT][BATCH_SLOT_VERT_CNT]f32,
	batch_tex_gl_id:      u32,
	view_mat:             Matrix_4x4,
}

Textures :: struct {
	gl_ids: []u32,
	sizes:  []Vec_2D_I,
}

Texture_Index_To_File_Path :: proc(index: int) -> cstring

Fonts :: struct {
	arrangement_infos: []Font_Arrangement_Info,
	tex_gl_ids:        []u32,
	tex_heights:       []int,
}

Font_Arrangement_Info :: struct {
	chr_hor_offsets:  [FONT_CHR_RANGE_LEN]int,
	chr_ver_offsets:  [FONT_CHR_RANGE_LEN]int,
	chr_hor_advances: [FONT_CHR_RANGE_LEN]int,
	chr_src_rects:    [FONT_CHR_RANGE_LEN]Rect_I,
	line_height:      int,
}

Font_Load_Info :: struct {
	file_path: string,
	height:    int,
}

Font_Index_To_Load_Info :: proc(index: int) -> Font_Load_Info

Batch_Shader_Prog :: struct {
	gl_id:                u32,
	proj_uniform_loc:     int,
	view_uniform_loc:     int,
	textures_uniform_loc: int,
}

Batch_GL_IDs :: struct {
	vert_array_gl_id: u32,
	vert_buf_gl_id:   u32,
	elem_buf_gl_id:   u32,
}

Str_Hor_Align :: enum {
	Left,
	Center,
	Right,
}

Str_Ver_Align :: enum {
	Top,
	Center,
	Bottom,
}

Str_Render_Info :: struct {
	chr_rects:  []Rect,
	line_infos: []Str_Line_Render_Info,
}

Str_Line_Render_Info :: struct {
	begin_chr_index:      int,
	width_including_offs: int,
}

gen_pers_render_data :: proc() -> Pers_Render_Data {
	return {load_batch_shader_prog(), gen_batch()}
}

clean_pers_render_data :: proc(render_data: ^Pers_Render_Data) {
	gl.DeleteVertexArrays(1, &render_data.batch_gl_ids.vert_array_gl_id)
	gl.DeleteBuffers(1, &render_data.batch_gl_ids.vert_buf_gl_id)
	gl.DeleteBuffers(1, &render_data.batch_gl_ids.elem_buf_gl_id)

	gl.DeleteProgram(render_data.batch_shader_prog.gl_id)

	rt.mem_zero(render_data, size_of(render_data^))
}

create_shader_from_src :: proc(src: cstring, frag: bool) -> u32 {
	shader_type: u32

	if frag {
		shader_type = gl.FRAGMENT_SHADER
	} else {
		shader_type = gl.VERTEX_SHADER
	}

	gl_id := gl.CreateShader(shader_type)

	src := src
	gl.ShaderSource(gl_id, 1, &src, nil)
	gl.CompileShader(gl_id)

	compile_success: i32
	gl.GetShaderiv(gl_id, gl.COMPILE_STATUS, &compile_success)

	if compile_success == 0 {
		gl.DeleteShader(gl_id)
		return 0
	}

	return gl_id
}

create_shader_prog_from_srcs :: proc(vert_shader_src, frag_shader_src: cstring) -> u32 {
	vert_shader_gl_id := create_shader_from_src(vert_shader_src, false)

	if vert_shader_gl_id == 0 {
		return 0
	}

	frag_shader_gl_id := create_shader_from_src(frag_shader_src, true)

	if frag_shader_gl_id == 0 {
		gl.DeleteShader(vert_shader_gl_id)
		return 0
	}

	prog_gl_id := gl.CreateProgram()
	gl.AttachShader(prog_gl_id, vert_shader_gl_id)
	gl.AttachShader(prog_gl_id, frag_shader_gl_id)
	gl.LinkProgram(prog_gl_id)

	gl.DeleteShader(vert_shader_gl_id)
	gl.DeleteShader(frag_shader_gl_id)

	return prog_gl_id
}

load_batch_shader_prog :: proc() -> Batch_Shader_Prog {
	vert_shader_src: cstring = `#version 430 core
layout (location = 0) in vec2 a_vert;
layout (location = 1) in vec2 a_pos;
layout (location = 2) in vec2 a_size;
layout (location = 3) in float a_rot;
layout (location = 4) in vec2 a_tex_coord;
layout (location = 5) in vec4 a_blend;

out vec2 v_tex_coord;
out vec4 v_blend;

uniform mat4 u_view;
uniform mat4 u_proj;

void main() {
    float rot_cos = cos(a_rot);
    float rot_sin = -sin(a_rot);

    mat4 model = mat4(
        vec4(a_size.x * rot_cos, a_size.x * rot_sin, 0.0, 0.0),
        vec4(a_size.y * -rot_sin, a_size.y * rot_cos, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(a_pos.x, a_pos.y, 0.0, 1.0)
    );

    gl_Position = u_proj * u_view * model * vec4(a_vert, 0.0, 1.0);
    v_tex_coord = a_tex_coord;
    v_blend = a_blend;
}`


	frag_shader_src: cstring = `#version 430 core
in vec2 v_tex_coord;
in vec4 v_blend;

out vec4 o_frag_color;

uniform sampler2D u_tex;

void main() {
    vec4 tex_color = texture(u_tex, v_tex_coord);
    o_frag_color = tex_color * v_blend;
}`


	prog: Batch_Shader_Prog
	prog.gl_id = create_shader_prog_from_srcs(vert_shader_src, frag_shader_src)
	assert(prog.gl_id != 0)

	prog.proj_uniform_loc = int(gl.GetUniformLocation(prog.gl_id, "u_proj"))
	prog.view_uniform_loc = int(gl.GetUniformLocation(prog.gl_id, "u_view"))
	prog.textures_uniform_loc = int(gl.GetUniformLocation(prog.gl_id, "u_textures"))

	return prog
}

gen_batch :: proc() -> Batch_GL_IDs {
	gl_ids: Batch_GL_IDs

	// Generate vertex array.
	gl.GenVertexArrays(1, &gl_ids.vert_array_gl_id)
	gl.BindVertexArray(gl_ids.vert_array_gl_id)

	// Generate vertex buffer.
	gl.GenBuffers(1, &gl_ids.vert_buf_gl_id)
	gl.BindBuffer(gl.ARRAY_BUFFER, gl_ids.vert_buf_gl_id)
	gl.BufferData(gl.ARRAY_BUFFER, BATCH_SLOT_VERTS_SIZE * BATCH_SLOT_CNT, nil, gl.DYNAMIC_DRAW)

	// Generate element buffer.
	gl.GenBuffers(1, &gl_ids.elem_buf_gl_id)
	gl.BindBuffer(gl.ELEMENT_ARRAY_BUFFER, gl_ids.elem_buf_gl_id)

	indices: [BATCH_SLOT_ELEM_CNT * BATCH_SLOT_CNT]u16

	for i := 0; i < BATCH_SLOT_CNT; i += 1 {
		indices[(i * 6) + 0] = u16((i * 4) + 0)
		indices[(i * 6) + 1] = u16((i * 4) + 1)
		indices[(i * 6) + 2] = u16((i * 4) + 2)
		indices[(i * 6) + 3] = u16((i * 4) + 2)
		indices[(i * 6) + 4] = u16((i * 4) + 3)
		indices[(i * 6) + 5] = u16((i * 4) + 0)
	}

	gl.BufferData(gl.ELEMENT_ARRAY_BUFFER, size_of(indices), &indices, gl.STATIC_DRAW)

	// Set vertex attribute pointers.
	verts_stride: i32 = size_of(f32) * BATCH_SHADER_PROG_VERT_CNT

	gl.VertexAttribPointer(0, 2, gl.FLOAT, gl.FALSE, verts_stride, size_of(f32) * 0)
	gl.EnableVertexAttribArray(0)

	gl.VertexAttribPointer(1, 2, gl.FLOAT, gl.FALSE, verts_stride, size_of(f32) * 2)
	gl.EnableVertexAttribArray(1)

	gl.VertexAttribPointer(2, 2, gl.FLOAT, gl.FALSE, verts_stride, size_of(f32) * 4)
	gl.EnableVertexAttribArray(2)

	gl.VertexAttribPointer(3, 1, gl.FLOAT, gl.FALSE, verts_stride, size_of(f32) * 6)
	gl.EnableVertexAttribArray(3)

	gl.VertexAttribPointer(4, 2, gl.FLOAT, gl.FALSE, verts_stride, size_of(f32) * 7)
	gl.EnableVertexAttribArray(4)

	gl.VertexAttribPointer(5, 4, gl.FLOAT, gl.FALSE, verts_stride, size_of(f32) * 9)
	gl.EnableVertexAttribArray(5)

	return gl_ids
}

load_textures :: proc(
	allocator: mem.Allocator,
	tex_cnt: int,
	tex_index_to_file_path_func: Texture_Index_To_File_Path,
) -> (
	Textures,
	bool,
) {
	assert(tex_cnt > 0)
	assert(tex_index_to_file_path_func != nil)

	textures: Textures

	textures.gl_ids = make([]u32, tex_cnt, allocator)

	if textures.gl_ids == nil {
		return textures, false
	}

	textures.sizes = make([]Vec_2D_I, tex_cnt, allocator)

	if textures.sizes == nil {
		return textures, false
	}

	gl.GenTextures(i32(tex_cnt), &textures.gl_ids[0])

	for i in 0 ..< tex_cnt {
		fp := tex_index_to_file_path_func(i)
		assert(fp != nil)

		width, height, channel_cnt: i32
		px_data := stbi.load(fp, &width, &height, &channel_cnt, TEXTURE_CHANNEL_CNT)

		if px_data == nil {
			fmt.printf("Failed to load image with file path \"%s\"!\n", fp)
			return textures, false
		}

		defer stbi.image_free(px_data)

		textures.sizes[i] = {int(width), int(height)}

		gl.BindTexture(gl.TEXTURE_2D, textures.gl_ids[i])
		gl.TexParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
		gl.TexParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST)
		gl.TexImage2D(
			gl.TEXTURE_2D,
			0,
			gl.RGBA,
			width,
			height,
			0,
			gl.RGBA,
			gl.UNSIGNED_BYTE,
			px_data,
		)
	}

	return textures, true
}

unload_textures :: proc(textures: ^Textures) {
	if len(textures.gl_ids) > 0 {
		gl.DeleteTextures(i32(len(textures.gl_ids)), &textures.gl_ids[0])
	}

	mem.zero_item(textures)
}

get_texture_cnt :: proc(textures: ^Textures) -> int {
	return len(textures.gl_ids)
}

load_fonts :: proc(
	allocator: mem.Allocator,
	scratch_allocator: mem.Allocator,
	font_cnt: int,
	font_index_to_load_info: Font_Index_To_Load_Info,
) -> (
	Fonts,
	bool,
) {
	assert(font_cnt > 0)
	assert(font_index_to_load_info != nil)

	fonts: Fonts

	// Reserve memory for font data.
	fonts.arrangement_infos = make([]Font_Arrangement_Info, font_cnt, allocator)

	if fonts.arrangement_infos == nil {
		return fonts, false
	}

	fonts.tex_gl_ids = make([]u32, font_cnt, allocator)

	if fonts.tex_gl_ids == nil {
		return fonts, false
	}

	fonts.tex_heights = make([]int, font_cnt, allocator)

	if fonts.tex_heights == nil {
		return fonts, false
	}

	// Reserve temporary memory to use as working space to store pixel data for each font texture.
	px_data_scratch_space := make(
		[]u8,
		TEXTURE_CHANNEL_CNT * FONT_TEXTURE_WIDTH * FONT_TEXTURE_HEIGHT_LIMIT,
		scratch_allocator,
	)

	// Load each font.
	for i in 0 ..< font_cnt {
		font_load_info := font_index_to_load_info(i)
		assert(font_load_info.height > 0)

		font_file_data, font_file_read_err := os.read_entire_file_from_filename_or_err(
			font_load_info.file_path,
			scratch_allocator,
		)

		if font_file_read_err != nil {
			return fonts, false
		}

		font_info: stbtt.fontinfo

		if !stbtt.InitFont(
			&font_info,
			&font_file_data[0],
			stbtt.GetFontOffsetForIndex(&font_file_data[0], 0),
		) {
			return fonts, false
		}

		scale := stbtt.ScaleForPixelHeight(&font_info, f32(font_load_info.height))

		ascent, descent, line_gap: i32
		stbtt.GetFontVMetrics(&font_info, &ascent, &descent, &line_gap)

		fonts.arrangement_infos[i].line_height = int(f32(ascent - descent + line_gap) * scale)

		for y in 0 ..< FONT_TEXTURE_HEIGHT_LIMIT {
			for x in 0 ..< FONT_TEXTURE_WIDTH {
				px_index := ((y * FONT_TEXTURE_WIDTH) + x) * TEXTURE_CHANNEL_CNT

				// Initialise to transparent white.
				px_data_scratch_space[px_index + 0] = 255
				px_data_scratch_space[px_index + 1] = 255
				px_data_scratch_space[px_index + 2] = 255
				px_data_scratch_space[px_index + 3] = 0
			}
		}

		chr_render_pos: Vec_2D_I

		for j in 0 ..< FONT_CHR_RANGE_LEN {
			chr := rune(FONT_CHR_RANGE_BEGIN + j)

			advance: i32
			stbtt.GetCodepointHMetrics(&font_info, chr, &advance, nil)

			fonts.arrangement_infos[i].chr_hor_advances[j] = int(f32(advance) * scale)

			if chr == ' ' {
				continue
			}

			bitmap_width, bitmap_height: i32
			bitmap_offs_x, bitmap_offs_y: i32

			bitmap_raw := stbtt.GetCodepointBitmap(
				&font_info,
				0,
				scale,
				chr,
				&bitmap_width,
				&bitmap_height,
				&bitmap_offs_x,
				&bitmap_offs_y,
			)

			if bitmap_raw == nil {
				return fonts, false
			}

			defer stbtt.FreeBitmap(bitmap_raw, nil)

			bitmap := mem.slice_ptr(
				bitmap_raw,
				int(bitmap_width * bitmap_height * TEXTURE_CHANNEL_CNT),
			)

			fonts.arrangement_infos[i].chr_hor_offsets[j] = int(bitmap_offs_x)
			fonts.arrangement_infos[i].chr_ver_offsets[j] =
				int(bitmap_offs_y) + int(f32(ascent) * scale)

			if chr_render_pos.x + int(bitmap_width) > FONT_TEXTURE_WIDTH {
				chr_render_pos.x = 0
				chr_render_pos.y += fonts.arrangement_infos[i].line_height
			}

			fonts.tex_heights[i] = max(fonts.tex_heights[i], chr_render_pos.y + int(bitmap_height))

			if fonts.tex_heights[i] > FONT_TEXTURE_HEIGHT_LIMIT {
				return fonts, false
			}

			fonts.arrangement_infos[i].chr_src_rects[j] = {
				chr_render_pos.x,
				chr_render_pos.y,
				int(bitmap_width),
				int(bitmap_height),
			}

			for y in 0 ..< int(bitmap_height) {
				for x in 0 ..< int(bitmap_width) {
					px_pos := Vec_2D_I{chr_render_pos.x + x, chr_render_pos.y + y}
					px_index := ((px_pos.y * FONT_TEXTURE_WIDTH) + px_pos.x) * TEXTURE_CHANNEL_CNT

					bitmap_index := (y * int(bitmap_width)) + x
					px_data_scratch_space[px_index + 3] = bitmap[bitmap_index]
				}
			}

			chr_render_pos.x += int(bitmap_width)
		}

		gl.BindTexture(gl.TEXTURE_2D, fonts.tex_gl_ids[i])
		gl.TexParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
		gl.TexParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST)
		gl.TexImage2D(
			gl.TEXTURE_2D,
			0,
			gl.RGBA,
			FONT_TEXTURE_WIDTH,
			i32(fonts.tex_heights[i]),
			0,
			gl.RGBA,
			gl.UNSIGNED_BYTE,
			&px_data_scratch_space[0],
		)
	}

	return fonts, true
}

unload_fonts :: proc(fonts: ^Fonts) {
	if len(fonts.tex_gl_ids) > 0 {
		gl.DeleteTextures(i32(len(fonts.tex_gl_ids)), &fonts.tex_gl_ids[0])
	}

	mem.zero_item(fonts)
}

get_font_cnt :: proc(fonts: ^Fonts) -> int {
	return len(fonts.arrangement_infos)
}

begin_rendering :: proc(state: ^Rendering_State) {
	mem.zero_item(state)
	init_iden_matrix_4x4(&state.view_mat)
}

render_clear :: proc(col: Vec_4D) {
	gl.ClearColor(col.x, col.y, col.z, col.w)
	gl.Clear(gl.COLOR_BUFFER_BIT)
}

render :: proc(
	rendering_context: ^Rendering_Context,
	tex_gl_id: u32,
	tex_coords: Rect_Edges,
	pos: Vec_2D,
	size: Vec_2D,
	origin := Vec_2D{0.5, 0.5},
	rot: f32 = 0.0,
	blend := WHITE,
) {
	if rendering_context.state.batch_slots_used_cnt == 0 {
		rendering_context.state.batch_tex_gl_id = tex_gl_id
	} else if rendering_context.state.batch_slots_used_cnt == BATCH_SLOT_CNT ||
	   tex_gl_id != rendering_context.state.batch_tex_gl_id {
		flush(rendering_context)
		render(rendering_context, tex_gl_id, tex_coords, pos, size, origin, rot, blend)
		return
	}

	// Submit the vertex data to the batch.
	slot_index := rendering_context.state.batch_slots_used_cnt
	slot_verts := &rendering_context.state.batch_slot_verts[slot_index]

	slot_verts[0] = 0.0 - origin.x
	slot_verts[1] = 0.0 - origin.y
	slot_verts[2] = pos.x
	slot_verts[3] = pos.y
	slot_verts[4] = size.x
	slot_verts[5] = size.y
	slot_verts[6] = rot
	slot_verts[7] = tex_coords.left
	slot_verts[8] = tex_coords.top
	slot_verts[9] = blend.x
	slot_verts[10] = blend.y
	slot_verts[11] = blend.z
	slot_verts[12] = blend.w

	slot_verts[13] = 1.0 - origin.x
	slot_verts[14] = 0.0 - origin.y
	slot_verts[15] = pos.x
	slot_verts[16] = pos.y
	slot_verts[17] = size.x
	slot_verts[18] = size.y
	slot_verts[19] = rot
	slot_verts[20] = tex_coords.right
	slot_verts[21] = tex_coords.top
	slot_verts[22] = blend.x
	slot_verts[23] = blend.y
	slot_verts[24] = blend.z
	slot_verts[25] = blend.w

	slot_verts[26] = 1.0 - origin.x
	slot_verts[27] = 1.0 - origin.y
	slot_verts[28] = pos.x
	slot_verts[29] = pos.y
	slot_verts[30] = size.x
	slot_verts[31] = size.y
	slot_verts[32] = rot
	slot_verts[33] = tex_coords.right
	slot_verts[34] = tex_coords.bottom
	slot_verts[35] = blend.x
	slot_verts[36] = blend.y
	slot_verts[37] = blend.z
	slot_verts[38] = blend.w

	slot_verts[39] = 0.0 - origin.x
	slot_verts[40] = 1.0 - origin.y
	slot_verts[41] = pos.x
	slot_verts[42] = pos.y
	slot_verts[43] = size.x
	slot_verts[44] = size.y
	slot_verts[45] = rot
	slot_verts[46] = tex_coords.left
	slot_verts[47] = tex_coords.bottom
	slot_verts[48] = blend.x
	slot_verts[49] = blend.y
	slot_verts[50] = blend.z
	slot_verts[51] = blend.w

	rendering_context.state.batch_slots_used_cnt += 1
}

render_texture :: proc(
	rendering_context: ^Rendering_Context,
	tex_index: int,
	textures: ^Textures,
	src_rect: Rect_I,
	pos: Vec_2D,
	origin := Vec_2D{0.5, 0.5},
	scale := Vec_2D{1.0, 1.0},
	rot: f32 = 0.0,
	blend := WHITE,
) {
	assert(tex_index >= 0 && tex_index < get_texture_cnt(textures))

	tex_size := textures.sizes[tex_index]
	tex_coords := calc_texture_coords(src_rect, tex_size)
	render(
		rendering_context,
		textures.gl_ids[tex_index],
		tex_coords,
		pos,
		{f32(src_rect.width) * scale.x, f32(src_rect.height) * scale.y},
		origin,
		rot,
		blend,
	)
}

render_str :: proc(
	rendering_context: ^Rendering_Context,
	str: string,
	font_index: int,
	fonts: ^Fonts,
	pos: Vec_2D,
	scratch_allocator: mem.Allocator,
	hor_align := Str_Hor_Align.Center,
	ver_align := Str_Ver_Align.Center,
	blend := WHITE,
) -> bool {
	render_info, render_info_generated := gen_str_render_info(
		str,
		scratch_allocator,
		font_index,
		fonts,
		pos,
		hor_align,
		ver_align,
	)

	if !render_info_generated {
		return false
	}

	font_tex_gl_id := fonts.tex_gl_ids[font_index]
	font_tex_size := Vec_2D_I{FONT_TEXTURE_WIDTH, fonts.tex_heights[font_index]}

	for i in 0 ..< len(render_info.chr_rects) {
		if str[i] == ' ' {
			continue
		}

		chr_index := str[i] - FONT_CHR_RANGE_BEGIN

		chr_tex_coords := calc_texture_coords(
			fonts.arrangement_infos[font_index].chr_src_rects[chr_index],
			font_tex_size,
		)
		chr_pos := calc_rect_pos(render_info.chr_rects[i])
		chr_size := calc_rect_size(render_info.chr_rects[i])

		render(
			rendering_context,
			fonts.tex_gl_ids[font_index],
			chr_tex_coords,
			chr_pos,
			chr_size,
			{},
			0.0,
			blend,
		)
	}

	return true
}

flush :: proc(rendering_context: ^Rendering_Context) {
	if rendering_context.state.batch_slots_used_cnt == 0 {
		return
	}

	// Write the batch vertex data to the GPU.
	gl.BindVertexArray(rendering_context.pers.batch_gl_ids.vert_array_gl_id)
	gl.BindBuffer(gl.ARRAY_BUFFER, rendering_context.pers.batch_gl_ids.vert_buf_gl_id)

	write_size := BATCH_SLOT_VERTS_SIZE * rendering_context.state.batch_slots_used_cnt
	gl.BufferSubData(
		gl.ARRAY_BUFFER,
		0,
		int(write_size),
		&rendering_context.state.batch_slot_verts[0][0],
	)

	// Draw the batch.
	prog := &rendering_context.pers.batch_shader_prog

	gl.UseProgram(prog.gl_id)

	proj_mat: Matrix_4x4
	init_ortho_matrix_4x4(
		&proj_mat,
		0.0,
		f32(rendering_context.display_size.x),
		f32(rendering_context.display_size.y),
		0.0,
		-1.0,
		1.0,
	)

	gl.UniformMatrix4fv(i32(prog.proj_uniform_loc), 1, false, &proj_mat.elems[0][0])
	gl.UniformMatrix4fv(
		i32(prog.view_uniform_loc),
		1,
		false,
		&rendering_context.state.view_mat.elems[0][0],
	)

	gl.BindTexture(gl.TEXTURE_2D, rendering_context.state.batch_tex_gl_id)

	gl.BindBuffer(gl.ELEMENT_ARRAY_BUFFER, rendering_context.pers.batch_gl_ids.elem_buf_gl_id)
	gl.DrawElements(
		gl.TRIANGLES,
		i32(BATCH_SLOT_ELEM_CNT * rendering_context.state.batch_slots_used_cnt),
		gl.UNSIGNED_SHORT,
		nil,
	)

	// Reset batch state.
	rendering_context.state.batch_slots_used_cnt = 0
	rendering_context.state.batch_tex_gl_id = 0
}

calc_texture_coords :: proc(src_rect: Rect_I, tex_size: Vec_2D_I) -> Rect_Edges {
	assert(src_rect.width > 0 && src_rect.height > 0 && tex_size.x > 0 && tex_size.y > 0)

	return {
		f32(src_rect.x) / f32(tex_size.x),
		f32(src_rect.y) / f32(tex_size.y),
		f32(calc_rect_i_right(src_rect)) / f32(tex_size.x),
		f32(calc_rect_i_bottom(src_rect)) / f32(tex_size.y),
	}
}

gen_str_render_info :: proc(
	str: string,
	allocator: mem.Allocator,
	font_index: int,
	fonts: ^Fonts,
	pos: Vec_2D,
	hor_align: Str_Hor_Align,
	ver_align: Str_Ver_Align,
) -> (
	Str_Render_Info,
	bool,
) {
	assert(font_index >= 0 && font_index < get_font_cnt(fonts))

	str_len := len(str)
	assert(str_len > 0)

	line_cnt := cnt_num_lines(str)

	render_info: Str_Render_Info

	render_info.chr_rects = make([]Rect, str_len, allocator)

	if render_info.chr_rects == nil {
		return render_info, false
	}

	render_info.line_infos = make([]Str_Line_Render_Info, line_cnt, allocator)

	if render_info.line_infos == nil {
		return render_info, false
	}

	//
	// First Phase: Determining the bases of character rectangles and also line information.
	//
	chr_render_pos_pen: Vec_2D_I = {}
	line_index := 0

	for i in 0 ..< str_len {
		chr := str[i]

		if chr != '\n' {
			chr_index := int(chr) - FONT_CHR_RANGE_BEGIN

			offs := Vec_2D_I {
				fonts.arrangement_infos[font_index].chr_hor_offsets[chr_index],
				fonts.arrangement_infos[font_index].chr_ver_offsets[chr_index],
			}

			chr_render_pos := Vec_2D_I {
				chr_render_pos_pen.x + offs.x,
				chr_render_pos_pen.y + offs.y,
			}

			render_info.chr_rects[i] = {
				f32(chr_render_pos.x),
				f32(chr_render_pos.y),
				f32(fonts.arrangement_infos[font_index].chr_src_rects[chr_index].width),
				f32(fonts.arrangement_infos[font_index].chr_src_rects[chr_index].height),
			}

			chr_render_pos_pen.x += fonts.arrangement_infos[font_index].chr_hor_advances[chr_index]
		} else {
			render_info.line_infos[line_index].width_including_offs = chr_render_pos_pen.x
			line_index += 1
			render_info.line_infos[line_index].begin_chr_index = i

			chr_render_pos_pen.x = 0
			chr_render_pos_pen.y += fonts.arrangement_infos[font_index].line_height
		}
	}

	render_info.line_infos[line_index].width_including_offs = chr_render_pos_pen.x

	//
	// Second Phase: Applying position and alignment offsets.
	//
	height_including_offs := chr_render_pos_pen.y + fonts.arrangement_infos[font_index].line_height
	ver_align_offs := -(f32(height_including_offs) * f32(ver_align) * 0.5)

	for i in 0 ..< line_cnt {
		line_end_chr_index: int

		if i < line_cnt - 1 {
			line_end_chr_index = render_info.line_infos[i + 1].begin_chr_index
		} else {
			line_end_chr_index = len(render_info.chr_rects)
		}

		hor_align_offs := -(f32(render_info.line_infos[i].width_including_offs) *
			f32(hor_align) *
			0.5)

		for j := render_info.line_infos[i].begin_chr_index; j < line_end_chr_index; j += 1 {
			translate_rect(
				&render_info.chr_rects[j],
				{pos.x + hor_align_offs, pos.y + ver_align_offs},
			)
		}
	}

	return render_info, true
}
