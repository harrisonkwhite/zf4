#include <zf4_rendering.h>

#include <zf4_utils.h>

namespace zf4::rendering {
    static void LoadTexturedQuadShaderProg(s_textured_quad_shader_prog& prog) {
        assert(IsStructZero(prog));

        const char* const vert_shader_src = R"(#version 430 core

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
        vec4(a_size.x * rot_cos, a_size.x * rot_sin, 0.0f, 0.0f),
        vec4(a_size.y * -rot_sin, a_size.y * rot_cos, 0.0f, 0.0f),
        vec4(0.0f, 0.0f, 1.0f, 0.0f),
        vec4(a_pos.x, a_pos.y, 0.0f, 1.0f)
    );

    gl_Position = u_proj * u_view * model * vec4(a_vert, 0.0f, 1.0f);

    v_tex_coord = a_tex_coord;
    v_blend = a_blend;
}
)";

        const char* const frag_shader_src = R"(#version 430 core

in vec2 v_tex_coord;
in vec4 v_blend;

out vec4 o_frag_color;

uniform sampler2D u_tex;

void main() {
    vec4 tex_color = texture(u_tex, v_tex_coord);
    o_frag_color = tex_color * v_blend;
}
)";

        prog.gl_id = CreateShaderProgFromSrcs(vert_shader_src, frag_shader_src);
        assert(prog.gl_id);

        prog.proj_uniform_loc = ZF4_GL_CALL(glGetUniformLocation(prog.gl_id, "u_proj"));
        prog.view_uniform_loc = ZF4_GL_CALL(glGetUniformLocation(prog.gl_id, "u_view"));
        prog.textures_uniform_loc = ZF4_GL_CALL(glGetUniformLocation(prog.gl_id, "u_textures"));
    }

    static bool AttachFramebufferTexture(const GLuint fb_gl_id, const GLuint tex_gl_id, const s_vec_2d_i tex_size) {
        assert(tex_gl_id);
        assert(fb_gl_id);
        assert(tex_size.x > 0 && tex_size.y > 0);

        ZF4_GL_CALL(glBindTexture(GL_TEXTURE_2D, tex_gl_id));
        ZF4_GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_size.x, tex_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
        ZF4_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        ZF4_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

        ZF4_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, fb_gl_id));

        ZF4_GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_gl_id, 0));

        const bool success = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

        ZF4_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));

        return success;
    }

    s_pers_render_data* LoadPersRenderData(s_mem_arena& mem_arena, s_mem_arena& scratch_space) {
        // Reserve memory for the pers_render_data.
        const auto pers_render_data = PushType<s_pers_render_data>(mem_arena);

        if (!pers_render_data) {
            LogError("Failed to reserve memory for persistent render data!");
            return nullptr;
        }

        // Load the textured quad shader program, used for render batches.
        LoadTexturedQuadShaderProg(pers_render_data->textured_quad_shader_prog);

        //
        // Surfaces
        //
        ZF4_GL_CALL(glGenVertexArrays(1, &pers_render_data->surf_vert_array_gl_id));
        ZF4_GL_CALL(glBindVertexArray(pers_render_data->surf_vert_array_gl_id));

        ZF4_GL_CALL(glGenBuffers(1, &pers_render_data->surf_vert_buf_gl_id));
        ZF4_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, pers_render_data->surf_vert_buf_gl_id));

        {
            const float verts[] = {
                -1.0f, -1.0f, 0.0f, 0.0f,
                1.0f, -1.0f, 1.0f, 0.0f,
                1.0f, 1.0f, 1.0f, 1.0f,
                -1.0f, 1.0f, 0.0f, 1.0f
            };

            ZF4_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW));
        }

        ZF4_GL_CALL(glGenBuffers(1, &pers_render_data->surf_elem_buf_gl_id));
        ZF4_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pers_render_data->surf_elem_buf_gl_id));

        {
            const unsigned short indices[] = {0, 1, 2, 2, 3, 0};
            ZF4_GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));
        }

        ZF4_GL_CALL(glEnableVertexAttribArray(0));
        ZF4_GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(float) * 4, (const void*)(sizeof(float) * 0)));

        ZF4_GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(float) * 4, (const void*)(sizeof(float) * 2)));
        ZF4_GL_CALL(glEnableVertexAttribArray(1));

        ZF4_GL_CALL(glBindVertexArray(0));

        //
        // Texture Batch Generation
        //

        // Generate vertex array.
        ZF4_GL_CALL(glGenVertexArrays(1, &pers_render_data->tex_batch_vert_array_gl_id));
        ZF4_GL_CALL(glBindVertexArray(pers_render_data->tex_batch_vert_array_gl_id));

        // Generate vertex buffer.
        ZF4_GL_CALL(glGenBuffers(1, &pers_render_data->tex_batch_vert_buf_gl_id));
        ZF4_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, pers_render_data->tex_batch_vert_buf_gl_id));
        ZF4_GL_CALL(glBufferData(GL_ARRAY_BUFFER, g_texture_batch_slot_verts_size * g_texture_batch_slot_limit, nullptr, GL_DYNAMIC_DRAW));

        // Generate element buffer.
        ZF4_GL_CALL(glGenBuffers(1, &pers_render_data->tex_batch_elem_buf_gl_id));
        ZF4_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pers_render_data->tex_batch_elem_buf_gl_id));

        {
            const auto indices = PushArray<unsigned short>(g_texture_batch_slot_indices_cnt * g_texture_batch_slot_limit, scratch_space);

            if (IsStructZero(indices)) {
                LogError("Failed to reserve memory for texture batch indices!");
                return nullptr;
            }

            for (int i = 0; i < g_texture_batch_slot_limit; i++) {
                indices[(i * 6) + 0] = static_cast<unsigned short>((i * 4) + 0);
                indices[(i * 6) + 1] = static_cast<unsigned short>((i * 4) + 1);
                indices[(i * 6) + 2] = static_cast<unsigned short>((i * 4) + 2);
                indices[(i * 6) + 3] = static_cast<unsigned short>((i * 4) + 2);
                indices[(i * 6) + 4] = static_cast<unsigned short>((i * 4) + 3);
                indices[(i * 6) + 5] = static_cast<unsigned short>((i * 4) + 0);
            }

            ZF4_GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, ArraySizeInBytes(static_cast<s_array<const unsigned short>>(indices)), indices.elems_raw, GL_STATIC_DRAW));
        }

        // Set vertex attribute pointers.
        const int verts_stride = sizeof(float) * g_textured_quad_shader_prog_cnt;

        ZF4_GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, false, verts_stride, reinterpret_cast<const void*>(sizeof(float) * 0)));
        ZF4_GL_CALL(glEnableVertexAttribArray(0));

        ZF4_GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, false, verts_stride, reinterpret_cast<const void*>(sizeof(float) * 2)));
        ZF4_GL_CALL(glEnableVertexAttribArray(1));

        ZF4_GL_CALL(glVertexAttribPointer(2, 2, GL_FLOAT, false, verts_stride, reinterpret_cast<const void*>(sizeof(float) * 4)));
        ZF4_GL_CALL(glEnableVertexAttribArray(2));

        ZF4_GL_CALL(glVertexAttribPointer(3, 1, GL_FLOAT, false, verts_stride, reinterpret_cast<const void*>(sizeof(float) * 6)));
        ZF4_GL_CALL(glEnableVertexAttribArray(3));

        ZF4_GL_CALL(glVertexAttribPointer(4, 2, GL_FLOAT, false, verts_stride, reinterpret_cast<const void*>(sizeof(float) * 7)));
        ZF4_GL_CALL(glEnableVertexAttribArray(4));

        ZF4_GL_CALL(glVertexAttribPointer(5, 4, GL_FLOAT, false, verts_stride, reinterpret_cast<const void*>(sizeof(float) * 9)));
        ZF4_GL_CALL(glEnableVertexAttribArray(5));

        return pers_render_data;
    }

    void CleanPersRenderData(s_pers_render_data& pers_render_data) {
        CleanSurfaces(pers_render_data.surfs);

        ZF4_GL_CALL(glDeleteBuffers(1, &pers_render_data.tex_batch_elem_buf_gl_id));
        ZF4_GL_CALL(glDeleteBuffers(1, &pers_render_data.tex_batch_vert_buf_gl_id));
        ZF4_GL_CALL(glDeleteVertexArrays(1, &pers_render_data.tex_batch_vert_array_gl_id));

        ZF4_GL_CALL(glDeleteBuffers(1, &pers_render_data.surf_elem_buf_gl_id));
        ZF4_GL_CALL(glDeleteBuffers(1, &pers_render_data.surf_vert_buf_gl_id));
        ZF4_GL_CALL(glDeleteVertexArrays(1, &pers_render_data.surf_vert_array_gl_id));

        ZeroOutStruct(pers_render_data);
    }

    bool InitSurfaces(const int cnt, s_surfaces& surfs, const s_vec_2d_i window_size) {
        assert(cnt > 0 && cnt <= g_surface_limit);
        assert(IsStructZero(surfs));
        assert(window_size.x > 0 && window_size.y > 0);

        surfs.cnt = cnt;

        ZF4_GL_CALL(glGenFramebuffers(cnt, surfs.framebuffer_gl_ids.elems_raw));
        ZF4_GL_CALL(glGenTextures(cnt, surfs.framebuffer_tex_gl_ids.elems_raw));

        for (int i = 0; i < cnt; ++i) {
            if (!AttachFramebufferTexture(surfs.framebuffer_gl_ids[i], surfs.framebuffer_tex_gl_ids[i], window_size)) {
                return false;
            }
        }

        return true;
    }

    void CleanSurfaces(s_surfaces& surfs) {
        ZF4_GL_CALL(glDeleteTextures(surfs.cnt, surfs.framebuffer_tex_gl_ids.elems_raw));
        ZF4_GL_CALL(glDeleteFramebuffers(surfs.cnt, surfs.framebuffer_gl_ids.elems_raw));

        ZeroOutStruct(surfs);
    }

    bool ResizeSurfaces(s_surfaces& surfs, const s_vec_2d_i window_size) {
        assert(window_size.x > 0 && window_size.y > 0);

        if (surfs.cnt > 0) {
            glDeleteTextures(surfs.cnt, surfs.framebuffer_tex_gl_ids.elems_raw);
            glGenTextures(surfs.cnt, surfs.framebuffer_tex_gl_ids.elems_raw);

            for (int i = 0; i < surfs.cnt; ++i) {
                if (!AttachFramebufferTexture(surfs.framebuffer_gl_ids[i], surfs.framebuffer_tex_gl_ids[i], window_size)) {
                    return false;
                }
            }
        }

        return true;
    }

    s_draw_phase_state* BeginDrawPhase(s_mem_arena& mem_arena, const s_vec_2d_i window_size) {
        const auto phase_state = PushType<s_draw_phase_state>(mem_arena);

        if (phase_state) {
            phase_state->proj_mat = GenOrthoMatrix4x4(0.0f, static_cast<float>(window_size.x), static_cast<float>(window_size.y), 0.0f, -1.0f, 1.0f); // NOTE: We can potentially cache this and only change on window resize.
            phase_state->view_mat = GenIdentityMatrix4x4();
        }

        ZF4_GL_CALL(glEnable(GL_BLEND));
        ZF4_GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        return phase_state;
    }

    void Clear(const s_vec_4d col) {
        glClearColor(col.x, col.y, col.z, col.w);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void SetSurface(const int surf_index, s_draw_phase_state& draw_phase_state, const s_pers_render_data& pers_render_data) {
        assert(surf_index >= 0 && surf_index < pers_render_data.surfs.cnt);

#if _DEBUG
        for (int i = 0; i < draw_phase_state.surf_index_stack.len; ++i) {
            if (draw_phase_state.surf_index_stack[i] == surf_index) {
                assert(false && "Attempting to set a render surface that has already been set!");
            }
        }
#endif

        // Add the surface index to the stack.
        ListAppend(draw_phase_state.surf_index_stack, surf_index);

        // Bind the surface framebuffer.
        ZF4_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, pers_render_data.surfs.framebuffer_gl_ids[surf_index]));
    }

    void UnsetSurface(s_draw_phase_state& draw_phase_state, const s_pers_render_data& pers_render_data) {
        assert(draw_phase_state.tex_batch_slots_used_cnt == 0); // Make sure that the texture batch has been flushed before unsetting the surface.

        ListPop(draw_phase_state.surf_index_stack);

        if (IsListEmpty(draw_phase_state.surf_index_stack)) {
            ZF4_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
        } else {
            const int new_surf_index = ListEnd(draw_phase_state.surf_index_stack);
            const GLuint fb_gl_id = pers_render_data.surfs.framebuffer_gl_ids[new_surf_index];
            ZF4_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, pers_render_data.surfs.framebuffer_gl_ids[new_surf_index]));
        }
    }

    void SetSurfaceShaderProg(const int prog_index, const s_shader_progs& progs, s_draw_phase_state& draw_phase_state) {
        assert(prog_index >= 0 && prog_index < progs.cnt);
        assert(draw_phase_state.tex_batch_slots_used_cnt == 0); // Make sure that the texture batch has been flushed before setting a new surface.
        draw_phase_state.surf_shader_prog_gl_id = progs.gl_ids[prog_index];
    }

    void SetSurfaceShaderProgUniform(const char* const uni_name, const u_shader_uniform_val val, const e_shader_uniform_val_type val_type, s_draw_phase_state& draw_phase_state) {
        assert(draw_phase_state.surf_shader_prog_gl_id);

        glUseProgram(draw_phase_state.surf_shader_prog_gl_id);

        const int uni_loc = glGetUniformLocation(draw_phase_state.surf_shader_prog_gl_id, uni_name);
        assert(uni_loc != -1);

        switch (val_type) {
            case ek_shader_uniform_val_type_int:
                ZF4_GL_CALL(glUniform1i(uni_loc, val.i));
                break;

            case ek_shader_uniform_val_type_float:
                ZF4_GL_CALL(glUniform1f(uni_loc, val.f));
                break;

            case ek_shader_uniform_val_type_v2:
                ZF4_GL_CALL(glUniform2fv(uni_loc, 1, reinterpret_cast<const float*>(&val.v2)));
                break;

            case ek_shader_uniform_val_type_v3:
                ZF4_GL_CALL(glUniform3fv(uni_loc, 1, reinterpret_cast<const float*>(&val.v3)));
                break;

            case ek_shader_uniform_val_type_v4:
                ZF4_GL_CALL(glUniform4fv(uni_loc, 1, reinterpret_cast<const float*>(&val.v4)));
                break;

            case ek_shader_uniform_val_type_mat4x4:
                ZF4_GL_CALL(glUniformMatrix4fv(uni_loc, 1, false, reinterpret_cast<const float*>(&val.mat4x4)));
                break;

            default:
                assert(false && "Invalid shader uniform value type provided!");
                break;
        }
    }

    void DrawSurface(const int surf_index, s_draw_phase_state& draw_phase_state, const s_pers_render_data& pers_render_data) {
        assert(surf_index >= 0 && surf_index < pers_render_data.surfs.cnt);
        assert(draw_phase_state.surf_shader_prog_gl_id); // Make sure the surface shader program has been set.

        ZF4_GL_CALL(glUseProgram(draw_phase_state.surf_shader_prog_gl_id));

        ZF4_GL_CALL(glActiveTexture(GL_TEXTURE0));
        ZF4_GL_CALL(glBindTexture(GL_TEXTURE_2D, pers_render_data.surfs.framebuffer_tex_gl_ids[surf_index]));

        ZF4_GL_CALL(glBindVertexArray(pers_render_data.surf_vert_array_gl_id));
        ZF4_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pers_render_data.surf_elem_buf_gl_id));
        ZF4_GL_CALL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr));

        // Reset the surface shader program. NOTE: Not sure if this should be kept?
        draw_phase_state.surf_shader_prog_gl_id = 0;
    }

    void Draw(const GLuint tex_gl_id, const s_rect_edges tex_coords, const s_vec_2d pos, const s_vec_2d size, s_draw_phase_state& draw_phase_state, const s_pers_render_data& pers_render_data, const s_vec_2d origin, const float rot, const s_vec_4d blend) {
        if (draw_phase_state.tex_batch_slots_used_cnt == 0) {
            draw_phase_state.tex_batch_tex_gl_id = tex_gl_id;
        } else if (draw_phase_state.tex_batch_slots_used_cnt == g_texture_batch_slot_limit || tex_gl_id != draw_phase_state.tex_batch_tex_gl_id) {
            Flush(draw_phase_state, pers_render_data);
            Draw(tex_gl_id, tex_coords, pos, size, draw_phase_state, pers_render_data, origin, rot, blend);
            return;
        }

        // Submit the vertex data to the batch.
        const int slot_index = draw_phase_state.tex_batch_slots_used_cnt;
        auto& slot_verts = draw_phase_state.tex_batch_slot_verts[slot_index];

        slot_verts[0] = 0.0f - origin.x;
        slot_verts[1] = 0.0f - origin.y;
        slot_verts[2] = pos.x;
        slot_verts[3] = pos.y;
        slot_verts[4] = size.x;
        slot_verts[5] = size.y;
        slot_verts[6] = rot;
        slot_verts[7] = tex_coords.left;
        slot_verts[8] = tex_coords.top;
        slot_verts[9] = blend.x;
        slot_verts[10] = blend.y;
        slot_verts[11] = blend.z;
        slot_verts[12] = blend.w;

        slot_verts[13] = 1.0f - origin.x;
        slot_verts[14] = 0.0f - origin.y;
        slot_verts[15] = pos.x;
        slot_verts[16] = pos.y;
        slot_verts[17] = size.x;
        slot_verts[18] = size.y;
        slot_verts[19] = rot;
        slot_verts[20] = tex_coords.right;
        slot_verts[21] = tex_coords.top;
        slot_verts[22] = blend.x;
        slot_verts[23] = blend.y;
        slot_verts[24] = blend.z;
        slot_verts[25] = blend.w;

        slot_verts[26] = 1.0f - origin.x;
        slot_verts[27] = 1.0f - origin.y;
        slot_verts[28] = pos.x;
        slot_verts[29] = pos.y;
        slot_verts[30] = size.x;
        slot_verts[31] = size.y;
        slot_verts[32] = rot;
        slot_verts[33] = tex_coords.right;
        slot_verts[34] = tex_coords.bottom;
        slot_verts[35] = blend.x;
        slot_verts[36] = blend.y;
        slot_verts[37] = blend.z;
        slot_verts[38] = blend.w;

        slot_verts[39] = 0.0f - origin.x;
        slot_verts[40] = 1.0f - origin.y;
        slot_verts[41] = pos.x;
        slot_verts[42] = pos.y;
        slot_verts[43] = size.x;
        slot_verts[44] = size.y;
        slot_verts[45] = rot;
        slot_verts[46] = tex_coords.left;
        slot_verts[47] = tex_coords.bottom;
        slot_verts[48] = blend.x;
        slot_verts[49] = blend.y;
        slot_verts[50] = blend.z;
        slot_verts[51] = blend.w;

        ++draw_phase_state.tex_batch_slots_used_cnt;
    }

    void DrawTexture(const int tex_index, const s_textures& textures, const s_rect_i src_rect, const s_vec_2d pos, s_draw_phase_state& draw_phase_state, const s_pers_render_data& pers_render_data, const s_vec_2d origin, const s_vec_2d scale, const float rot, const s_vec_4d blend) {
        assert(tex_index >= 0 && tex_index < textures.cnt);

        const s_vec_2d_i tex_size = textures.sizes[tex_index];
        const s_rect_edges tex_coords = CalcTexCoords(src_rect, tex_size);
        Draw(textures.gl_ids[tex_index], tex_coords, pos, {src_rect.width * scale.x, src_rect.height * scale.y}, draw_phase_state, pers_render_data, origin, rot, blend);
    }

    void DrawStr(const char* const str, const int font_index, const s_fonts& fonts, const s_vec_2d pos, const s_vec_4d blend, const e_str_hor_align hor_align, const e_str_ver_align ver_align, s_draw_phase_state& draw_phase_state, const s_pers_render_data& pers_render_data) {
        assert(str);
        assert(font_index >= 0 && font_index < fonts.cnt);

        const s_str_draw_info str_draw_info = LoadStrDrawInfo(str, font_index, fonts);

        const s_font_arrangement_info& font_arrangement_info = fonts.arrangement_infos[font_index];
        const GLuint font_tex_gl_id = fonts.tex_gl_ids[font_index];
        const s_vec_2d_i font_tex_size = fonts.tex_sizes[font_index];

        int line_index = 0;

        for (int i = 0; str[i]; ++i) {
            if (str[i] == '\n') {
                ++line_index;
                continue;
            }

            if (str[i] == ' ') {
                continue;
            }

            const int char_index = str[i] - g_font_char_range_begin;

            // Note that the character position is offset based on alignment.
            // Middle alignment (1) corresponds to 50% multiplier on current line width and text height, right alignment (2) is 100%.
            const s_vec_2d char_pos = {
                pos.x + str_draw_info.char_draw_positions[i].x - (str_draw_info.line_widths[line_index] * hor_align * 0.5f),
                pos.y + str_draw_info.char_draw_positions[i].y - (str_draw_info.height * ver_align * 0.5f)
            };

            const s_rect_i char_src_rect = font_arrangement_info.chars.src_rects[char_index];

            Draw(font_tex_gl_id, CalcTexCoords(char_src_rect, font_tex_size), char_pos, RectSize(char_src_rect), draw_phase_state, pers_render_data, {}, 0.0f, blend);
        }
    }

    void DrawRect(const s_rect rect, const s_vec_4d blend, s_draw_phase_state& draw_phase_state, const s_pers_render_data& pers_render_data, const s_builtin_assets& builtin_assets) {
        Draw(builtin_assets.pixel_tex_gl_id, {0.0f, 0.0f, 1.0f, 1.0f}, RectTopLeft(rect), RectSize(rect), draw_phase_state, pers_render_data, {}, 0.0f, blend);
    }

    void DrawRectOutline(const s_rect rect, const s_vec_4d blend, s_draw_phase_state& draw_phase_state, const s_pers_render_data& pers_render_data, const s_builtin_assets& builtin_assets) {
        DrawLine(RectTopLeft(rect), RectTopRight(rect), 1, blend, draw_phase_state, pers_render_data, builtin_assets);
        DrawLine(RectTopRight(rect), RectBottomRight(rect), 1, blend, draw_phase_state, pers_render_data, builtin_assets);
        DrawLine(RectBottomRight(rect), RectBottomLeft(rect), 1, blend, draw_phase_state, pers_render_data, builtin_assets);
        DrawLine(RectBottomLeft(rect), RectTopLeft(rect), 1, blend, draw_phase_state, pers_render_data, builtin_assets);
    }

    void DrawLine(const s_vec_2d start, const s_vec_2d end, const float width, const s_vec_4d blend, s_draw_phase_state& draw_phase_state, const s_pers_render_data& pers_render_data, const s_builtin_assets& builtin_assets) {
        assert(width > 0.0f);

        const float len = Dist(start, end);
        const float rot = Dir(start, end);
        Draw(builtin_assets.pixel_tex_gl_id, {0.0f, 0.0f, 1.0f, 1.0f}, start, {len, width}, draw_phase_state, pers_render_data, {0.0f, 0.5f}, rot, blend);
    }

    void DrawBar(const s_vec_2d pos, const s_vec_2d size, const float perc, const s_vec_3d col_front, const s_vec_3d col_back, const float alpha, s_draw_phase_state& draw_phase_state, const s_pers_render_data& pers_render_data, const s_builtin_assets& builtin_assets) {
        assert(perc >= 0.0f && perc <= 1.0f);
        assert(alpha >= 0.0f, && alpha <= 1.0f);

        // TODO: Support different bar directions.

        const s_vec_2d topleft = pos - (size / 2.0f);
        DrawRect({topleft.x, topleft.y, size.x * perc, size.y}, {col_front.x, col_front.y, col_front.z, alpha}, draw_phase_state, pers_render_data, builtin_assets);
        DrawRect({topleft.x + (size.x * perc), topleft.y, size.x * (1.0f - perc), size.y}, {col_back.x, col_back.y, col_back.z, alpha}, draw_phase_state, pers_render_data, builtin_assets);
    }

    void Flush(s_draw_phase_state& draw_phase_state, const s_pers_render_data& pers_render_data) {
        if (draw_phase_state.tex_batch_slots_used_cnt == 0) {
            return;
        }

        // Write the batch vertex data to the GPU.
        ZF4_GL_CALL(glBindVertexArray(pers_render_data.tex_batch_vert_array_gl_id));
        ZF4_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, pers_render_data.tex_batch_vert_buf_gl_id));

        const int write_size = g_texture_batch_slot_verts_size * draw_phase_state.tex_batch_slots_used_cnt;
        ZF4_GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, write_size, draw_phase_state.tex_batch_slot_verts.elems_raw));

        // Draw the batch.
        const s_textured_quad_shader_prog& prog = pers_render_data.textured_quad_shader_prog;

        ZF4_GL_CALL(glUseProgram(prog.gl_id));

        ZF4_GL_CALL(glUniformMatrix4fv(prog.proj_uniform_loc, 1, false, reinterpret_cast<const float*>(&draw_phase_state.proj_mat)));
        ZF4_GL_CALL(glUniformMatrix4fv(prog.view_uniform_loc, 1, false, reinterpret_cast<const float*>(&draw_phase_state.view_mat)));

        ZF4_GL_CALL(glBindTexture(GL_TEXTURE_2D, draw_phase_state.tex_batch_tex_gl_id));

        ZF4_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pers_render_data.tex_batch_elem_buf_gl_id));
        ZF4_GL_CALL(glDrawElements(GL_TRIANGLES, g_texture_batch_slot_indices_cnt * draw_phase_state.tex_batch_slots_used_cnt, GL_UNSIGNED_SHORT, nullptr));

        // Clear batch state.
        memset(draw_phase_state.tex_batch_slot_verts.elems_raw, 0, sizeof(draw_phase_state.tex_batch_slot_verts.elems_raw));
        draw_phase_state.tex_batch_slots_used_cnt = 0;
        draw_phase_state.tex_batch_tex_gl_id = 0;
    }

    s_str_draw_info LoadStrDrawInfo(const char* const str, const int font_index, const s_fonts& fonts) {
        s_str_draw_info draw_info = {};

        const int str_len = strlen(str);
        assert(str_len > 0 && str_len <= g_str_draw_len_limit);

        const s_font_arrangement_info& font_arrangement_info = fonts.arrangement_infos[font_index];

        // Iterate through each character and set draw positions, meanwhile determining line widths, the overall text height, etc.
        const int space_char_index = ' ' - g_font_char_range_begin; // We use this character for defaulting in some cases.

        s_vec_2d_i char_draw_pos_pen = {};

        int line_index = 0;

        int first_line_min_ver_offs;
        bool first_line_min_ver_offs_defined = false;

        const int last_line_max_height_default = font_arrangement_info.chars.ver_offsets[space_char_index] + font_arrangement_info.chars.src_rects[space_char_index].height;
        int last_line_max_height = last_line_max_height_default;
        bool last_line_max_height_updated = false; // We want to let the max height initially be overwritten regardless of the default.

        for (int i = 0; i < str_len; i++) {
            const int char_index = str[i] - g_font_char_range_begin;

            if (str[i] == '\n') {
                draw_info.line_widths[line_index] = char_draw_pos_pen.x; // The width of this line is where we've horizontally drawn up to.

                // Reset the last line max height (this didn't turn out to be the last line).
                last_line_max_height = last_line_max_height_default;
                last_line_max_height_updated = false; // Let the max height be overwritten by the first character of the next line.

                // Move the pen to the next line.
                char_draw_pos_pen.x = 0;
                char_draw_pos_pen.y += font_arrangement_info.line_height;

                ++line_index;

                continue;
            }

            // For all characters after the first, apply kerning.
            if (i > 0) {
                const int str_char_index_last = str[i - 1] - g_font_char_range_begin;
                const int kerning_index = (char_index * g_font_char_range_len) + str_char_index_last;
                char_draw_pos_pen.x += font_arrangement_info.chars.kernings[kerning_index];
            }

            // Set the top-left position to draw this character.
            const s_vec_2d_i char_draw_pos = {
                char_draw_pos_pen.x + font_arrangement_info.chars.hor_offsets[char_index],
                char_draw_pos_pen.y + font_arrangement_info.chars.ver_offsets[char_index]
            };

            draw_info.char_draw_positions[i] = char_draw_pos;

            // Move to the next character.
            char_draw_pos_pen.x += font_arrangement_info.chars.hor_advances[char_index];

            // If this is the first line, update the minimum vertical offset.
            if (line_index == 0) {
                if (!first_line_min_ver_offs_defined) {
                    first_line_min_ver_offs = font_arrangement_info.chars.ver_offsets[char_index];
                    first_line_min_ver_offs_defined = true;
                } else {
                    first_line_min_ver_offs = Min(font_arrangement_info.chars.ver_offsets[char_index], first_line_min_ver_offs);
                }
            }

            // Update the maximum height. Note that we aren't sure if this is the last line.
            const int height = font_arrangement_info.chars.ver_offsets[char_index] + font_arrangement_info.chars.src_rects[char_index].height;

            if (!last_line_max_height_updated) {
                last_line_max_height = height;
                last_line_max_height_updated = true;
            } else {
                last_line_max_height_updated = Max(height, last_line_max_height);
            }
        }

        // Set the width of the final line.
        draw_info.line_widths[line_index] = char_draw_pos_pen.x;

        // If the minimum vertical offset of the first line wasn't set (because the first line was empty), just use the space character.
        // Note that we don't want this vertical offset to affect the minimum calculation, hence why it was not assigned as default.
        if (!first_line_min_ver_offs_defined) {
            first_line_min_ver_offs = font_arrangement_info.chars.ver_offsets[space_char_index];
            first_line_min_ver_offs_defined = true;
        }

        draw_info.char_cnt = str_len;
        draw_info.line_cnt = line_index + 1;

        draw_info.height = first_line_min_ver_offs + char_draw_pos_pen.y + last_line_max_height;

        return draw_info;
    }

    s_rect_edges CalcTexCoords(const s_rect_i src_rect, const s_vec_2d_i tex_size) {
        return {
            static_cast<float>(src_rect.x) / tex_size.x,
            static_cast<float>(src_rect.y) / tex_size.y,
            static_cast<float>(RectRight(src_rect)) / tex_size.x,
            static_cast<float>(RectBottom(src_rect)) / tex_size.y
        };
    }
}
