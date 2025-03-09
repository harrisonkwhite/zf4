package zf4

import "core:fmt"
import "core:mem"
import rt "base:runtime"
import gl "vendor:OpenGL"
import stbi "vendor:stb/image"

TEXTURE_CHANNEL_CNT :: 4

TEXTURED_QUAD_SHADER_PROG_VERT_CNT :: 13

BATCH_SLOT_CNT :: 2048
BATCH_SLOT_VERT_CNT :: 44
BATCH_SLOT_VERTS_SIZE :: BATCH_SLOT_VERT_CNT * BATCH_SLOT_CNT

Textures :: struct {
	gl_ids: []u32,
	sizes: []Size_2D,
	cnt: u32
}

Pers_Render_Data :: struct {
	textures: Textures,
	batch_gl_ids: Batch_GL_IDs
}

Batch_GL_IDs :: struct {
	vert_array_gl_id: u32,
	vert_buf_gl_id: u32,
	elem_buf_gl_id: u32
}

Draw_Phase_State :: struct {
	batch_slots_used_cnt: u32,
	batch_slot_verts: [BATCH_SLOT_CNT][44]f32,

	view_mat: Matrix_4x4
}

Texture_Index_To_File_Path :: proc(index: u32) -> cstring

init_textures :: proc(textures: ^Textures, allocator: mem.Allocator, tex_cnt: u32, tex_index_to_file_path: Texture_Index_To_File_Path) -> bool {
	assert(textures != nil)
	assert(tex_cnt > 0)
	assert(tex_index_to_file_path != nil)

	textures.gl_ids = make([]u32, tex_cnt, allocator)

    if textures.gl_ids == nil {
        return false
    }

	textures.sizes = make([]Size_2D, tex_cnt, allocator)

    if textures.sizes == nil {
        return false
    }

    gl.GenTextures(i32(tex_cnt), &textures.gl_ids[0])

    for i: u32 = 0; i < tex_cnt; i += 1 {
        fp := tex_index_to_file_path(i)
        assert(fp != nil)

		width, height, channel_cnt: i32
		px_data := stbi.load(fp, &width, &height, &channel_cnt, TEXTURE_CHANNEL_CNT)

        if px_data == nil {
			fmt.printf("Failed to load texture with file path \"%s\"!\n", fp);
			gl.DeleteTextures(i32(tex_cnt), &textures.gl_ids[0])
            return false
        }

        textures.sizes[i] = {u32(width), u32(height)}

        gl.BindTexture(gl.TEXTURE_2D, textures.gl_ids[i])
        gl.TexParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
        gl.TexParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST)
        gl.TexImage2D(gl.TEXTURE_2D, 0, gl.RGBA, width, height, 0, gl.RGBA, gl.UNSIGNED_BYTE, px_data)

        stbi.image_free(px_data)
    }

	return true
}

gen_pers_render_data :: proc(allocator: mem.Allocator, tex_cnt: u32, tex_index_to_file_path: Texture_Index_To_File_Path) -> (Pers_Render_Data, bool) {
	render_data: Pers_Render_Data

	if !init_textures(&render_data.textures, allocator, tex_cnt, tex_index_to_file_path) {
		return {}, false
	}

	render_data.batch_gl_ids = gen_batch()

	return render_data, true
}

clean_pers_render_data :: proc(render_data: ^Pers_Render_Data) {
	gl.DeleteVertexArrays(1, &render_data.batch_gl_ids.vert_array_gl_id)
	gl.DeleteBuffers(1, &render_data.batch_gl_ids.vert_buf_gl_id)
	gl.DeleteBuffers(1, &render_data.batch_gl_ids.elem_buf_gl_id)

	rt.mem_zero(render_data, size_of(render_data^))
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

	indices: [6 * BATCH_SLOT_CNT]u16

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
    verts_stride: i32 = size_of(f32) * TEXTURED_QUAD_SHADER_PROG_VERT_CNT

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

begin_draw_phase :: proc() -> ^Draw_Phase_State {
	phase_state := new(Draw_Phase_State)
	init_iden_matrix_4x4(&phase_state.view_mat)
	return phase_state
}

draw_clear :: proc(col: Vec_4D) {
	gl.ClearColor(0.2, 0.3, 0.3, 1.0)
	gl.Clear(gl.COLOR_BUFFER_BIT)
}
