#include <zf4_rendering.h>

#include <zf4_utils.h>

namespace zf4 {
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

    static void LoadStrDrawInfo(s_str_draw_info& draw_info, const char* const str, const int font_index, const s_fonts* const fonts) {
        assert(IsStructZero(draw_info));

        const int str_len = (int)strlen(str);
        assert(str_len > 0 && str_len <= g_str_draw_len_limit);

        const s_font_arrangement_info* const font_arrangement_info = &fonts->arrangement_infos[font_index];

        // Iterate through each character and set draw positions, meanwhile determining line widths, the overall text height, etc.
        const int space_char_index = ' ' - g_font_char_range_begin; // We use this character for defaulting in some cases.

        s_vec_2d_i char_draw_pos_pen = {};

        int line_index = 0;

        int first_line_min_ver_offs;
        bool first_line_min_ver_offs_defined = false;

        const int last_line_max_height_default = font_arrangement_info->chars.ver_offsets[space_char_index] + font_arrangement_info->chars.src_rects[space_char_index].height;
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
                char_draw_pos_pen.y += font_arrangement_info->line_height;

                ++line_index;

                continue;
            }

            // For all characters after the first, apply kerning.
            if (i > 0) {
                const int str_char_index_last = str[i - 1] - g_font_char_range_begin;
                const int kerning_index = (char_index * g_font_char_range_len) + str_char_index_last;
                char_draw_pos_pen.x += font_arrangement_info->chars.kernings[kerning_index];
            }

            // Set the top-left position to draw this character.
            const s_vec_2d_i char_draw_pos = {
                char_draw_pos_pen.x + font_arrangement_info->chars.hor_offsets[char_index],
                char_draw_pos_pen.y + font_arrangement_info->chars.ver_offsets[char_index]
            };

            draw_info.char_draw_positions[i] = char_draw_pos;

            // Move to the next character.
            char_draw_pos_pen.x += font_arrangement_info->chars.hor_advances[char_index];

            // If this is the first line, update the minimum vertical offset.
            if (line_index == 0) {
                if (!first_line_min_ver_offs_defined) {
                    first_line_min_ver_offs = font_arrangement_info->chars.ver_offsets[char_index];
                    first_line_min_ver_offs_defined = true;
                } else {
                    first_line_min_ver_offs = ZF4_MIN(font_arrangement_info->chars.ver_offsets[char_index], first_line_min_ver_offs);
                }
            }

            // Update the maximum height. Note that we aren't sure if this is the last line.
            const int height = font_arrangement_info->chars.ver_offsets[char_index] + font_arrangement_info->chars.src_rects[char_index].height;

            if (!last_line_max_height_updated) {
                last_line_max_height = height;
                last_line_max_height_updated = true;
            } else {
                last_line_max_height_updated = ZF4_MAX(height, last_line_max_height);
            }
        }

        // Set the width of the final line.
        draw_info.line_widths[line_index] = char_draw_pos_pen.x;

        // If the minimum vertical offset of the first line wasn't set (because the first line was empty), just use the space character.
        // Note that we don't want this vertical offset to affect the minimum calculation, hence why it was not assigned as default.
        if (!first_line_min_ver_offs_defined) {
            first_line_min_ver_offs = font_arrangement_info->chars.ver_offsets[space_char_index];
            first_line_min_ver_offs_defined = true;
        }

        draw_info.char_cnt = str_len;
        draw_info.line_cnt = line_index + 1;

        draw_info.height = first_line_min_ver_offs + char_draw_pos_pen.y + last_line_max_height;
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

    static void SubmitToRenderBatch(const s_vec_2d pos, const s_vec_2d origin, const s_vec_2d scale, const float rot, const GLuint tex_gl_id, const s_vec_2d_i tex_size, const s_rect_i src_rect, const s_vec_4d blend, s_draw_phase_state& draw_phase_state, const s_renderer& renderer) {
        if (draw_phase_state.tex_batch_slots_used_cnt == 0) {
            draw_phase_state.tex_batch_tex_gl_id = tex_gl_id;
        } else if (draw_phase_state.tex_batch_slots_used_cnt == g_texture_batch_slot_limit || tex_gl_id != draw_phase_state.tex_batch_tex_gl_id) {
            FlushTextureBatch(draw_phase_state, renderer);
            SubmitToRenderBatch(pos, origin, scale, rot, tex_gl_id, tex_size, src_rect, blend, draw_phase_state, renderer);
            return;
        }

        // Submit the vertex data to the batch.
        const s_rect_edges tex_coords = {
            (float)src_rect.x / tex_size.x,
            (float)src_rect.y / tex_size.y,
            (float)RectRight(src_rect) / tex_size.x,
            (float)RectBottom(src_rect) / tex_size.y
        };

        const int slot_index = draw_phase_state.tex_batch_slots_used_cnt;
        auto& slot_verts = draw_phase_state.tex_batch_slot_verts[slot_index];

        slot_verts[0] = (0.0f - origin.x) * scale.x;
        slot_verts[1] = (0.0f - origin.y) * scale.y;
        slot_verts[2] = pos.x;
        slot_verts[3] = pos.y;
        slot_verts[4] = (float)src_rect.width;
        slot_verts[5] = (float)src_rect.height;
        slot_verts[6] = rot;
        slot_verts[7] = tex_coords.left;
        slot_verts[8] = tex_coords.top;
        slot_verts[9] = blend.x;
        slot_verts[10] = blend.y;
        slot_verts[11] = blend.z;
        slot_verts[12] = blend.w;

        slot_verts[13] = (1.0f - origin.x) * scale.x;
        slot_verts[14] = (0.0f - origin.y) * scale.y;
        slot_verts[15] = pos.x;
        slot_verts[16] = pos.y;
        slot_verts[17] = (float)src_rect.width;
        slot_verts[18] = (float)src_rect.height;
        slot_verts[19] = rot;
        slot_verts[20] = tex_coords.right;
        slot_verts[21] = tex_coords.top;
        slot_verts[22] = blend.x;
        slot_verts[23] = blend.y;
        slot_verts[24] = blend.z;
        slot_verts[25] = blend.w;

        slot_verts[26] = (1.0f - origin.x) * scale.x;
        slot_verts[27] = (1.0f - origin.y) * scale.y;
        slot_verts[28] = pos.x;
        slot_verts[29] = pos.y;
        slot_verts[30] = (float)src_rect.width;
        slot_verts[31] = (float)src_rect.height;
        slot_verts[32] = rot;
        slot_verts[33] = tex_coords.right;
        slot_verts[34] = tex_coords.bottom;
        slot_verts[35] = blend.x;
        slot_verts[36] = blend.y;
        slot_verts[37] = blend.z;
        slot_verts[38] = blend.w;

        slot_verts[39] = (0.0f - origin.x) * scale.x;
        slot_verts[40] = (1.0f - origin.y) * scale.y;
        slot_verts[41] = pos.x;
        slot_verts[42] = pos.y;
        slot_verts[43] = (float)src_rect.width;
        slot_verts[44] = (float)src_rect.height;
        slot_verts[45] = rot;
        slot_verts[46] = tex_coords.left;
        slot_verts[47] = tex_coords.bottom;
        slot_verts[48] = blend.x;
        slot_verts[49] = blend.y;
        slot_verts[50] = blend.z;
        slot_verts[51] = blend.w;

        ++draw_phase_state.tex_batch_slots_used_cnt;
    }

    s_renderer* LoadRenderer(s_mem_arena& mem_arena, s_mem_arena& scratch_space) {
        // Reserve memory for the renderer.
        const auto renderer = PushType<s_renderer>(mem_arena);

        if (!renderer) {
            LogError("Failed to reserve memory for a renderer!");
            return nullptr;
        }

        // Load the textured quad shader program, used for render batches.
        LoadTexturedQuadShaderProg(renderer->textured_quad_shader_prog);

        //
        // Surfaces
        //
        ZF4_GL_CALL(glGenVertexArrays(1, &renderer->surf_vert_array_gl_id));
        ZF4_GL_CALL(glBindVertexArray(renderer->surf_vert_array_gl_id));

        ZF4_GL_CALL(glGenBuffers(1, &renderer->surf_vert_buf_gl_id));
        ZF4_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, renderer->surf_vert_buf_gl_id));

        {
            const float verts[] = {
                -1.0f, -1.0f, 0.0f, 0.0f,
                1.0f, -1.0f, 1.0f, 0.0f,
                1.0f, 1.0f, 1.0f, 1.0f,
                -1.0f, 1.0f, 0.0f, 1.0f
            };

            ZF4_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW));
        }

        ZF4_GL_CALL(glGenBuffers(1, &renderer->surf_elem_buf_gl_id));
        ZF4_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->surf_elem_buf_gl_id));

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
        ZF4_GL_CALL(glGenVertexArrays(1, &renderer->tex_batch_vert_array_gl_id));
        ZF4_GL_CALL(glBindVertexArray(renderer->tex_batch_vert_array_gl_id));

        // Generate vertex buffer.
        ZF4_GL_CALL(glGenBuffers(1, &renderer->tex_batch_vert_buf_gl_id));
        ZF4_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, renderer->tex_batch_vert_buf_gl_id));
        ZF4_GL_CALL(glBufferData(GL_ARRAY_BUFFER, g_texture_batch_slot_verts_size * g_texture_batch_slot_limit, nullptr, GL_DYNAMIC_DRAW));

        // Generate element buffer.
        ZF4_GL_CALL(glGenBuffers(1, &renderer->tex_batch_elem_buf_gl_id));
        ZF4_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->tex_batch_elem_buf_gl_id));

        {
            const auto indices = PushArray<unsigned short>(g_texture_batch_slot_indices_cnt * g_texture_batch_slot_limit, scratch_space);

            if (IsStructZero(indices)) {
                LogError("Failed to reserve memory for texture batch indices!");
                return nullptr;
            }

            for (int i = 0; i < g_texture_batch_slot_limit; i++) {
                indices[(i * 6) + 0] = (unsigned short)((i * 4) + 0);
                indices[(i * 6) + 1] = (unsigned short)((i * 4) + 1);
                indices[(i * 6) + 2] = (unsigned short)((i * 4) + 2);
                indices[(i * 6) + 3] = (unsigned short)((i * 4) + 2);
                indices[(i * 6) + 4] = (unsigned short)((i * 4) + 3);
                indices[(i * 6) + 5] = (unsigned short)((i * 4) + 0);
            }

            ZF4_GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, ArraySizeInBytes(CreateView(indices)), indices.elems_raw, GL_STATIC_DRAW));
        }

        // Set vertex attribute pointers.
        const int verts_stride = sizeof(float) * g_textured_quad_shader_prog_cnt;

        ZF4_GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, false, verts_stride, (const void*)(sizeof(float) * 0)));
        ZF4_GL_CALL(glEnableVertexAttribArray(0));

        ZF4_GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, false, verts_stride, (const void*)(sizeof(float) * 2)));
        ZF4_GL_CALL(glEnableVertexAttribArray(1));

        ZF4_GL_CALL(glVertexAttribPointer(2, 2, GL_FLOAT, false, verts_stride, (const void*)(sizeof(float) * 4)));
        ZF4_GL_CALL(glEnableVertexAttribArray(2));

        ZF4_GL_CALL(glVertexAttribPointer(3, 1, GL_FLOAT, false, verts_stride, (const void*)(sizeof(float) * 6)));
        ZF4_GL_CALL(glEnableVertexAttribArray(3));

        ZF4_GL_CALL(glVertexAttribPointer(4, 2, GL_FLOAT, false, verts_stride, (const void*)(sizeof(float) * 7)));
        ZF4_GL_CALL(glEnableVertexAttribArray(4));

        ZF4_GL_CALL(glVertexAttribPointer(5, 4, GL_FLOAT, false, verts_stride, (const void*)(sizeof(float) * 9)));
        ZF4_GL_CALL(glEnableVertexAttribArray(5));

        return renderer;
    }

    void CleanRenderer(s_renderer& renderer) {
        CleanRenderSurfaces(renderer.surfs);

        ZF4_GL_CALL(glDeleteBuffers(1, &renderer.tex_batch_elem_buf_gl_id));
        ZF4_GL_CALL(glDeleteBuffers(1, &renderer.tex_batch_vert_buf_gl_id));
        ZF4_GL_CALL(glDeleteVertexArrays(1, &renderer.tex_batch_vert_array_gl_id));

        ZF4_GL_CALL(glDeleteBuffers(1, &renderer.surf_elem_buf_gl_id));
        ZF4_GL_CALL(glDeleteBuffers(1, &renderer.surf_vert_buf_gl_id));
        ZF4_GL_CALL(glDeleteVertexArrays(1, &renderer.surf_vert_array_gl_id));

        ZeroOutStruct(renderer);
    }

    bool InitRenderSurfaces(const int cnt, s_render_surfaces& surfs, const s_vec_2d_i window_size) {
        assert(cnt > 0 && cnt <= g_render_surface_limit);
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

    void CleanRenderSurfaces(s_render_surfaces& surfs) {
        ZF4_GL_CALL(glDeleteTextures(surfs.cnt, surfs.framebuffer_tex_gl_ids.elems_raw));
        ZF4_GL_CALL(glDeleteFramebuffers(surfs.cnt, surfs.framebuffer_gl_ids.elems_raw));

        ZeroOutStruct(surfs);
    }

    bool ResizeRenderSurfaces(s_render_surfaces& surfs, const s_vec_2d_i window_size) {
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

    s_draw_phase_state* BeginDrawPhase(s_mem_arena& mem_arena, const s_vec_2d_i window_size, const s_assets& assets) {
        const auto phase_state = PushType<s_draw_phase_state>(mem_arena);

        if (phase_state) {
            InitOrthoMatrix4x4(phase_state->proj_mat, 0.0f, (float)window_size.x, (float)window_size.y, 0.0f, -1.0f, 1.0f); // NOTE: We can potentially cache this and only change on window resize.
            InitIdentityMatrix4x4(phase_state->view_mat);

            phase_state->assets = &assets; // NOTE: Remove?
        }

        ZF4_GL_CALL(glEnable(GL_BLEND));
        ZF4_GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        return phase_state;
    }

    void RenderClear(const s_vec_4d col) {
        glClearColor(col.x, col.y, col.z, col.w);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void SubmitTextureToRenderBatch(const int tex_index, const s_rect_i src_rect, const s_vec_2d pos, s_draw_phase_state& draw_phase_state, const s_renderer& renderer, const s_vec_2d origin, const s_vec_2d scale, const float rot, const s_vec_4d blend) {
        assert(tex_index >= 0 && tex_index < draw_phase_state.assets->textures.cnt);
        const GLuint tex_gl_id = draw_phase_state.assets->textures.gl_ids[tex_index];
        const s_vec_2d_i tex_size = draw_phase_state.assets->textures.sizes[tex_index];
        SubmitToRenderBatch(pos, origin, scale, rot, tex_gl_id, tex_size, src_rect, blend, draw_phase_state, renderer);
    }

    void SubmitStrToRenderBatch(const char* const str, const int font_index, const s_vec_2d pos, const s_vec_4d blend, const e_str_hor_align hor_align, const e_str_ver_align ver_align, s_draw_phase_state& draw_phase_state, const s_renderer& renderer) {
        assert(str);
        assert(font_index >= 0 && font_index < draw_phase_state.assets->fonts.cnt);

        s_str_draw_info str_draw_info = {}; // NOTE: Might want to push this to a memory arena instead.
        LoadStrDrawInfo(str_draw_info, str, font_index, &draw_phase_state.assets->fonts);

        const s_font_arrangement_info* const font_arrangement_info = &draw_phase_state.assets->fonts.arrangement_infos[font_index];
        const GLuint font_tex_gl_id = draw_phase_state.assets->fonts.tex_gl_ids[font_index];
        const s_vec_2d_i font_tex_size = draw_phase_state.assets->fonts.tex_sizes[font_index];

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
                pos.x + str_draw_info.char_draw_positions[i].x - (str_draw_info.line_widths[line_index] * (int)hor_align * 0.5f),
                pos.y + str_draw_info.char_draw_positions[i].y - (str_draw_info.height * (int)ver_align * 0.5f)
            };

            const s_rect_i char_src_rect = font_arrangement_info->chars.src_rects[char_index];

            SubmitToRenderBatch(char_pos, {}, {1.0f, 1.0f}, 0.0f, font_tex_gl_id, font_tex_size, char_src_rect, blend, draw_phase_state, renderer);
        }
    }

    void FlushTextureBatch(s_draw_phase_state& draw_phase_state, const s_renderer& renderer) {
        if (draw_phase_state.tex_batch_slots_used_cnt == 0) {
            return;
        }

        // Write the batch vertex data to the GPU.
        ZF4_GL_CALL(glBindVertexArray(renderer.tex_batch_vert_array_gl_id));
        ZF4_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, renderer.tex_batch_vert_buf_gl_id));

        const int write_size = g_texture_batch_slot_verts_size * draw_phase_state.tex_batch_slots_used_cnt;
        ZF4_GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, write_size, draw_phase_state.tex_batch_slot_verts.elems_raw));

        // Draw the batch.
        const s_textured_quad_shader_prog& prog = renderer.textured_quad_shader_prog;

        ZF4_GL_CALL(glUseProgram(prog.gl_id));

        ZF4_GL_CALL(glUniformMatrix4fv(prog.proj_uniform_loc, 1, false, (const float*)&draw_phase_state.proj_mat));
        ZF4_GL_CALL(glUniformMatrix4fv(prog.view_uniform_loc, 1, false, (const float*)&draw_phase_state.view_mat));

        ZF4_GL_CALL(glBindTexture(GL_TEXTURE_2D, draw_phase_state.tex_batch_tex_gl_id));

        //GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer.tex_batch_elem_buf_gl_id));
        ZF4_GL_CALL(glDrawElements(GL_TRIANGLES, g_texture_batch_slot_indices_cnt * draw_phase_state.tex_batch_slots_used_cnt, GL_UNSIGNED_SHORT, nullptr));

        // Clear batch state.
        ZeroOutArrayElems(draw_phase_state.tex_batch_slot_verts);
        draw_phase_state.tex_batch_slots_used_cnt = 0;
        draw_phase_state.tex_batch_tex_gl_id = 0;
    }

    void SetRenderSurface(const int surf_index, s_draw_phase_state& draw_phase_state, const s_renderer& renderer) {
        assert(surf_index >= 0 && surf_index < renderer.surfs.cnt);

#if _DEBUG
        for (int i = 0; i < draw_phase_state.surf_index_stack_height; ++i) {
            if (draw_phase_state.surf_index_stack[i] == surf_index) {
                assert(false && "Attempting to set a render surface that has already been set!");
            }
        }
#endif

    // Add the surface index to the stack.
        assert(draw_phase_state.surf_index_stack_height < g_render_surface_limit); // NOTE: Maybe replace with proper error?
        draw_phase_state.surf_index_stack[draw_phase_state.surf_index_stack_height] = surf_index;
        ++draw_phase_state.surf_index_stack_height;

        // Bind the surface framebuffer.
        ZF4_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, renderer.surfs.framebuffer_gl_ids[surf_index]));
    }

    void UnsetRenderSurface(s_draw_phase_state& draw_phase_state, const s_renderer& renderer) {
        assert(draw_phase_state.surf_index_stack_height > 0);
        assert(draw_phase_state.tex_batch_slots_used_cnt == 0); // Make sure that the texture batch has been flushed before unsetting the surface.

        --draw_phase_state.surf_index_stack_height;

        if (draw_phase_state.surf_index_stack_height > 0) {
            const int new_surf_index = draw_phase_state.surf_index_stack[draw_phase_state.surf_index_stack_height - 1];
            const GLuint fb_gl_id = renderer.surfs.framebuffer_gl_ids[new_surf_index];
            ZF4_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, fb_gl_id));
        } else {
            ZF4_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
        }
    }

    void SetRenderSurfaceShaderProg(const int shader_prog_index, s_draw_phase_state& draw_phase_state) {
        assert(shader_prog_index >= 0 && shader_prog_index < draw_phase_state.assets->shader_progs.cnt);
        assert(draw_phase_state.tex_batch_slots_used_cnt == 0); // Make sure that the texture batch has been flushed before setting a new surface.
        draw_phase_state.surf_shader_prog_gl_id = draw_phase_state.assets->shader_progs.gl_ids[shader_prog_index];
    }

    void SetRenderSurfaceShaderProgUniform(const char* const uni_name, const u_shader_uniform_val val, const e_shader_uniform_val_type val_type, s_draw_phase_state& draw_phase_state) {
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
                ZF4_GL_CALL(glUniform2fv(uni_loc, 1, (const float*)&val.v2));
                break;

            case ek_shader_uniform_val_type_v3:
                ZF4_GL_CALL(glUniform3fv(uni_loc, 1, (const float*)&val.v3));
                break;

            case ek_shader_uniform_val_type_v4:
                ZF4_GL_CALL(glUniform4fv(uni_loc, 1, (const float*)&val.v4));
                break;

            case ek_shader_uniform_val_type_mat4x4:
                ZF4_GL_CALL(glUniformMatrix4fv(uni_loc, 1, false, (const float*)&val.mat4x4));
                break;

            default:
                assert(false && "Invalid shader uniform value type provided!");
                break;
        }
    }

    void DrawRenderSurface(const int surf_index, s_draw_phase_state& draw_phase_state, const s_renderer& renderer) {
        assert(surf_index >= 0 && surf_index < renderer.surfs.cnt);
        assert(draw_phase_state.surf_shader_prog_gl_id); // Make sure the surface shader program has been set.

        ZF4_GL_CALL(glUseProgram(draw_phase_state.surf_shader_prog_gl_id));

        ZF4_GL_CALL(glActiveTexture(GL_TEXTURE0));
        ZF4_GL_CALL(glBindTexture(GL_TEXTURE_2D, renderer.surfs.framebuffer_tex_gl_ids[surf_index]));

        ZF4_GL_CALL(glBindVertexArray(renderer.surf_vert_array_gl_id));
        ZF4_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer.surf_elem_buf_gl_id));
        ZF4_GL_CALL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr));

        // Reset the surface shader program. NOTE: Not sure if this should be kept?
        draw_phase_state.surf_shader_prog_gl_id = 0;
    }
}
