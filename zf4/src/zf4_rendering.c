#include <zf4_rendering.h>

#include <zf4_utils.h>

static void LoadTexturedQuadShaderProg(s_textured_quad_shader_prog* const prog) {
    assert(IsClear(prog, sizeof(*prog)));

    const char* const vert_shader_src =
        "#version 430 core\n"
        "\n"
        "layout (location = 0) in vec2 a_vert;\n"
        "layout (location = 1) in vec2 a_pos;\n"
        "layout (location = 2) in vec2 a_size;\n"
        "layout (location = 3) in float a_rot;\n"
        "layout (location = 4) in vec2 a_tex_coord;\n"
        "layout (location = 5) in vec4 a_blend;\n"
        "\n"
        "out vec2 v_tex_coord;\n"
        "out vec4 v_blend;\n"
        "\n"
        "uniform mat4 u_view;\n"
        "uniform mat4 u_proj;\n"
        "\n"
        "void main() {\n"
        "    float rot_cos = cos(a_rot);\n"
        "    float rot_sin = -sin(a_rot);\n"
        "\n"
        "    mat4 model = mat4(\n"
        "        vec4(a_size.x * rot_cos, a_size.x * rot_sin, 0.0f, 0.0f),\n"
        "        vec4(a_size.y * -rot_sin, a_size.y * rot_cos, 0.0f, 0.0f),\n"
        "        vec4(0.0f, 0.0f, 1.0f, 0.0f),\n"
        "        vec4(a_pos.x, a_pos.y, 0.0f, 1.0f)\n"
        "    );\n"
        "\n"
        "    gl_Position = u_proj * u_view * model * vec4(a_vert, 0.0f, 1.0f);\n"
        "\n"
        "    v_tex_coord = a_tex_coord;\n"
        "    v_blend = a_blend;\n"
        "}\n";

    const char* const frag_shader_src =
        "#version 430 core\n"
        "\n"
        "in vec2 v_tex_coord;\n"
        "in vec4 v_blend;\n"
        "\n"
        "out vec4 o_frag_color;\n"
        "\n"
        "uniform sampler2D u_tex;\n"
        "\n"
        "void main() {\n"
        "    vec4 tex_color = texture(u_tex, v_tex_coord);\n"
        "    o_frag_color = tex_color * v_blend;\n"
        "}\n";

    prog->gl_id = CreateShaderProgFromSrcs(vert_shader_src, frag_shader_src);
    assert(prog->gl_id);

    prog->proj_uniform_loc = GL_CALL(glGetUniformLocation(prog->gl_id, "u_proj"));
    prog->view_uniform_loc = GL_CALL(glGetUniformLocation(prog->gl_id, "u_view"));
    prog->textures_uniform_loc = GL_CALL(glGetUniformLocation(prog->gl_id, "u_textures"));
}

static void LoadStrDrawInfo(s_str_draw_info* const draw_info, const char* const str, const int font_index, const s_fonts* const fonts) {
    assert(IsClear(draw_info, sizeof(*draw_info)));

    const int str_len = strlen(str);
    assert(str_len > 0 && str_len <= STR_DRAW_LEN_LIMIT);

    const s_font_arrangement_info* const font_arrangement_info = &fonts->arrangement_infos[font_index];

    // Iterate through each character and set draw positions, meanwhile determining line widths, the overall text height, etc.
    const int space_char_index = ' ' - FONT_CHAR_RANGE_BEGIN; // We use this character for defaulting in some cases.

    s_vec_2d_i char_draw_pos_pen = {0};

    int line_index = 0;

    int first_line_min_ver_offs;
    bool first_line_min_ver_offs_defined = false;

    const int last_line_max_height_default = font_arrangement_info->chars.ver_offsets[space_char_index] + font_arrangement_info->chars.src_rects[space_char_index].height;
    int last_line_max_height = last_line_max_height_default;
    bool last_line_max_height_updated = false; // We want to let the max height initially be overwritten regardless of the default.

    for (int i = 0; i < str_len; i++) {
        const int char_index = str[i] - FONT_CHAR_RANGE_BEGIN;

        if (str[i] == '\n') {
            draw_info->line_widths[line_index] = char_draw_pos_pen.x; // The width of this line is where we've horizontally drawn up to.

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
            const int str_char_index_last = str[i - 1] - FONT_CHAR_RANGE_BEGIN;
            const int kerning_index = (char_index * FONT_CHAR_RANGE_LEN) + str_char_index_last;
            char_draw_pos_pen.x += font_arrangement_info->chars.kernings[kerning_index];
        }

        // Set the top-left position to draw this character.
        const s_vec_2d_i char_draw_pos = {
            char_draw_pos_pen.x + font_arrangement_info->chars.hor_offsets[char_index],
            char_draw_pos_pen.y + font_arrangement_info->chars.ver_offsets[char_index]
        };

        draw_info->char_draw_positions[i] = char_draw_pos;

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
    draw_info->line_widths[line_index] = char_draw_pos_pen.x;

    // If the minimum vertical offset of the first line wasn't set (because the first line was empty), just use the space character.
    // Note that we don't want this vertical offset to affect the minimum calculation, hence why it was not assigned as default.
    if (!first_line_min_ver_offs_defined) {
        first_line_min_ver_offs = font_arrangement_info->chars.ver_offsets[space_char_index];
        first_line_min_ver_offs_defined = true;
    }

    draw_info->char_cnt = str_len;
    draw_info->line_cnt = line_index + 1;

    draw_info->height = first_line_min_ver_offs + char_draw_pos_pen.y + last_line_max_height;
}

static bool AttachFramebufferTexture(const GLuint fb_gl_id, const GLuint tex_gl_id, const s_vec_2d_i tex_size) {
    assert(tex_gl_id);
    assert(fb_gl_id);
    assert(tex_size.x > 0 && tex_size.y > 0);

    GL_CALL(glBindTexture(GL_TEXTURE_2D, tex_gl_id));
    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_size.x, tex_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, fb_gl_id));

    GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_gl_id, 0));

    const bool success = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    return success;
}

static void SubmitToRenderBatch(const s_vec_2d pos, const s_vec_2d origin, const s_vec_2d scale, const float rot, const GLuint tex_gl_id, const s_vec_2d_i tex_size, const s_rect_i src_rect, const s_vec_4d blend, s_draw_phase_state* const draw_phase_state, const s_renderer* const renderer) {
    if (draw_phase_state->tex_batch_slots_used_cnt == 0) {
        draw_phase_state->tex_batch_tex_gl_id = tex_gl_id;
    } else if (draw_phase_state->tex_batch_slots_used_cnt == TEXTURE_BATCH_SLOT_LIMIT || tex_gl_id != draw_phase_state->tex_batch_tex_gl_id) {
        FlushTextureBatch(draw_phase_state, renderer);
        SubmitToRenderBatch(pos, origin, scale, rot, tex_gl_id, tex_size, src_rect, blend, draw_phase_state, renderer);
        return;
    }

    // Submit the vertex data to the batch.
    const s_rect_edges tex_coords = {
        (float)src_rect.x / tex_size.x,
        (float)src_rect.y / tex_size.y,
        (float)RectRightI(&src_rect) / tex_size.x,
        (float)RectBottomI(&src_rect) / tex_size.y
    };

    const int slot_index = draw_phase_state->tex_batch_slots_used_cnt;
    float* const slot_verts = draw_phase_state->tex_batch_slot_verts[slot_index];

    slot_verts[0] = (0.0f - origin.x) * scale.x;
    slot_verts[1] = (0.0f - origin.y) * scale.y;
    slot_verts[2] = pos.x;
    slot_verts[3] = pos.y;
    slot_verts[4] = src_rect.width;
    slot_verts[5] = src_rect.height;
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
    slot_verts[17] = src_rect.width;
    slot_verts[18] = src_rect.height;
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
    slot_verts[30] = src_rect.width;
    slot_verts[31] = src_rect.height;
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
    slot_verts[43] = src_rect.width;
    slot_verts[44] = src_rect.height;
    slot_verts[45] = rot;
    slot_verts[46] = tex_coords.left;
    slot_verts[47] = tex_coords.bottom;
    slot_verts[48] = blend.x;
    slot_verts[49] = blend.y;
    slot_verts[50] = blend.z;
    slot_verts[51] = blend.w;

    ++draw_phase_state->tex_batch_slots_used_cnt;
}

s_renderer* LoadRenderer(s_mem_arena* const mem_arena, s_mem_arena* const scratch_space) {
    assert(mem_arena);

    // Reserve memory for the renderer.
    s_renderer* const renderer = PushAligned(sizeof(*renderer), alignof(s_renderer), mem_arena);

    if (!renderer) {
        LogError("Failed to reserve memory for a renderer!");
        return NULL;
    }

    // Load the textured quad shader program, used for render batches.
    LoadTexturedQuadShaderProg(&renderer->textured_quad_shader_prog);

    //
    // Surfaces
    //
    GL_CALL(glGenVertexArrays(1, &renderer->surf_vert_array_gl_id));
    GL_CALL(glBindVertexArray(renderer->surf_vert_array_gl_id));

    GL_CALL(glGenBuffers(1, &renderer->surf_vert_buf_gl_id));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, renderer->surf_vert_buf_gl_id));

    {
        const float verts[] = {
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 0.0f, 1.0f
        };

        GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW));
    }

    GL_CALL(glGenBuffers(1, &renderer->surf_elem_buf_gl_id));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->surf_elem_buf_gl_id));

    {
        const unsigned short indices[] = {0, 1, 2, 2, 3, 0};
        GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));
    }

    GL_CALL(glEnableVertexAttribArray(0));
    GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(float) * 4, (const void*)(sizeof(float) * 0)));

    GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(float) * 4, (const void*)(sizeof(float) * 2)));
    GL_CALL(glEnableVertexAttribArray(1));

    GL_CALL(glBindVertexArray(0));

    //
    // Texture Batch Generation
    //

    // Generate vertex array.
    GL_CALL(glGenVertexArrays(1, &renderer->tex_batch_vert_array_gl_id));
    GL_CALL(glBindVertexArray(renderer->tex_batch_vert_array_gl_id));

    // Generate vertex buffer.
    GL_CALL(glGenBuffers(1, &renderer->tex_batch_vert_buf_gl_id));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, renderer->tex_batch_vert_buf_gl_id));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, TEXTURE_BATCH_SLOT_VERTS_SIZE * TEXTURE_BATCH_SLOT_LIMIT, NULL, GL_DYNAMIC_DRAW));

    // Generate element buffer.
    GL_CALL(glGenBuffers(1, &renderer->tex_batch_elem_buf_gl_id));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->tex_batch_elem_buf_gl_id));

    {
        const int indices_len = TEXTURE_BATCH_SLOT_INDICES_CNT * TEXTURE_BATCH_SLOT_LIMIT;
        unsigned short* const indices = PushAligned(sizeof(*indices) * indices_len, alignof(unsigned short), scratch_space);

        if (!indices) {
            LogError("Failed to reserve memory for texture batch indices!");
            return NULL;
        }

        for (int i = 0; i < TEXTURE_BATCH_SLOT_LIMIT; i++) {
            indices[(i * 6) + 0] = (i * 4) + 0;
            indices[(i * 6) + 1] = (i * 4) + 1;
            indices[(i * 6) + 2] = (i * 4) + 2;
            indices[(i * 6) + 3] = (i * 4) + 2;
            indices[(i * 6) + 4] = (i * 4) + 3;
            indices[(i * 6) + 5] = (i * 4) + 0;
        }

        GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*indices) * indices_len, indices, GL_STATIC_DRAW));
    }

    // Set vertex attribute pointers.
    const int verts_stride = sizeof(float) * TEXTURED_QUAD_SHADER_PROG_VERT_CNT;

    GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, false, verts_stride, (const void*)(sizeof(float) * 0)));
    GL_CALL(glEnableVertexAttribArray(0));

    GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, false, verts_stride, (const void*)(sizeof(float) * 2)));
    GL_CALL(glEnableVertexAttribArray(1));

    GL_CALL(glVertexAttribPointer(2, 2, GL_FLOAT, false, verts_stride, (const void*)(sizeof(float) * 4)));
    GL_CALL(glEnableVertexAttribArray(2));

    GL_CALL(glVertexAttribPointer(3, 1, GL_FLOAT, false, verts_stride, (const void*)(sizeof(float) * 6)));
    GL_CALL(glEnableVertexAttribArray(3));

    GL_CALL(glVertexAttribPointer(4, 2, GL_FLOAT, false, verts_stride, (const void*)(sizeof(float) * 7)));
    GL_CALL(glEnableVertexAttribArray(4));

    GL_CALL(glVertexAttribPointer(5, 4, GL_FLOAT, false, verts_stride, (const void*)(sizeof(float) * 9)));
    GL_CALL(glEnableVertexAttribArray(5));

    return renderer;
}

void CleanRenderer(s_renderer* const renderer) {
    assert(renderer);

    CleanRenderSurfaces(&renderer->surfs);

    GL_CALL(glDeleteBuffers(1, &renderer->tex_batch_elem_buf_gl_id));
    GL_CALL(glDeleteBuffers(1, &renderer->tex_batch_vert_buf_gl_id));
    GL_CALL(glDeleteVertexArrays(1, &renderer->tex_batch_vert_array_gl_id));

    GL_CALL(glDeleteBuffers(1, &renderer->surf_elem_buf_gl_id));
    GL_CALL(glDeleteBuffers(1, &renderer->surf_vert_buf_gl_id));
    GL_CALL(glDeleteVertexArrays(1, &renderer->surf_vert_array_gl_id));

    Clear(renderer, sizeof(*renderer));
}

bool InitRenderSurfaces(const int cnt, s_render_surfaces* const surfs, const s_vec_2d_i window_size) {
    assert(cnt > 0 && cnt <= RENDER_SURFACE_LIMIT);
    assert(surfs);
    assert(IsClear(surfs, sizeof(*surfs)));
    assert(window_size.x > 0 && window_size.y > 0);

    surfs->cnt = cnt;

    GL_CALL(glGenFramebuffers(cnt, surfs->framebuffer_gl_ids));
    GL_CALL(glGenTextures(cnt, surfs->framebuffer_tex_gl_ids));

    for (int i = 0; i < cnt; ++i) {
        if (!AttachFramebufferTexture(surfs->framebuffer_gl_ids[i], surfs->framebuffer_tex_gl_ids[i], window_size)) {
            return false;
        }
    }

    return true;
}

void CleanRenderSurfaces(s_render_surfaces* const surfs) {
    assert(surfs);

    GL_CALL(glDeleteTextures(surfs->cnt, surfs->framebuffer_tex_gl_ids));
    GL_CALL(glDeleteFramebuffers(surfs->cnt, surfs->framebuffer_gl_ids));

    Clear(surfs, sizeof(*surfs));
}

bool ResizeRenderSurfaces(s_render_surfaces* const surfs, const s_vec_2d_i window_size) {
    assert(surfs);
    assert(window_size.x > 0 && window_size.y > 0);

    if (surfs->cnt > 0) {
        glDeleteTextures(surfs->cnt, surfs->framebuffer_tex_gl_ids);
        glGenTextures(surfs->cnt, surfs->framebuffer_tex_gl_ids);

        for (int i = 0; i < surfs->cnt; ++i) {
            if (!AttachFramebufferTexture(surfs->framebuffer_gl_ids[i], surfs->framebuffer_tex_gl_ids[i], window_size)) {
                return false;
            }
        }
    }

    return true;
}

s_draw_phase_state* BeginDrawPhase(s_mem_arena* const mem_arena, const s_vec_2d_i window_size, const s_assets* const assets) {
    assert(window_size.x > 0 && window_size.y > 0);
    assert(assets);

    s_draw_phase_state* const phase_state = PushAligned(sizeof(*phase_state), alignof(s_draw_phase_state), mem_arena);

    if (phase_state) {
        InitOrthoMatrix4x4(&phase_state->proj_mat, 0.0f, window_size.x, window_size.y, 0.0f, -1.0f, 1.0f); // NOTE: We can potentially cache this and only change on window resize.
        InitIdentityMatrix4x4(&phase_state->view_mat);

        phase_state->assets = assets;
    }

    GL_CALL(glEnable(GL_BLEND));
    GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    return phase_state;
}

void RenderClear(const s_vec_4d col) {
    assert(IsColorValid(&col));

    glClearColor(col.x, col.y, col.z, col.w);
    glClear(GL_COLOR_BUFFER_BIT);
}

void SubmitTextureToRenderBatch(const int tex_index, const s_rect_i src_rect, const s_vec_2d pos, const s_vec_2d origin, const s_vec_2d scale, const float rot, const s_vec_4d blend, s_draw_phase_state* const draw_phase_state, const s_renderer* const renderer) {
    assert(tex_index >= 0 && tex_index < draw_phase_state->assets->textures.cnt);
    const GLuint tex_gl_id = draw_phase_state->assets->textures.gl_ids[tex_index];
    const s_vec_2d_i tex_size = draw_phase_state->assets->textures.sizes[tex_index];
    SubmitToRenderBatch(pos, origin, scale, rot, tex_gl_id, tex_size, src_rect, blend, draw_phase_state, renderer);
}

void SubmitStrToRenderBatch(const char* const str, const int font_index, const s_vec_2d pos, const s_vec_4d blend, const enum str_hor_align hor_align, const enum str_ver_align ver_align, s_draw_phase_state* const draw_phase_state, const s_renderer* const renderer) {
    assert(str);
    assert(font_index >= 0 && font_index < draw_phase_state->assets->fonts.cnt);
    assert(IsColorValid(&blend));
    assert(draw_phase_state);
    assert(renderer);

    s_str_draw_info str_draw_info = {0}; // NOTE: Might want to push this to a memory arena instead.
    LoadStrDrawInfo(&str_draw_info, str, font_index, &draw_phase_state->assets->fonts);

    const s_font_arrangement_info* const font_arrangement_info = &draw_phase_state->assets->fonts.arrangement_infos[font_index];
    const GLuint font_tex_gl_id = draw_phase_state->assets->fonts.tex_gl_ids[font_index];
    const s_vec_2d_i font_tex_size = draw_phase_state->assets->fonts.tex_sizes[font_index];

    int line_index = 0;

    for (int i = 0; str[i]; ++i) {
        if (str[i] == '\n') {
            ++line_index;
            continue;
        }

        if (str[i] == ' ') {
            continue;
        }

        const int char_index = str[i] - FONT_CHAR_RANGE_BEGIN;

        // Note that the character position is offset based on alignment.
        // Middle alignment (1) corresponds to 50% multiplier on current line width and text height, right alignment (2) is 100%.
        const s_vec_2d char_pos = {
            pos.x + str_draw_info.char_draw_positions[i].x - (str_draw_info.line_widths[line_index] * hor_align * 0.5f),
            pos.y + str_draw_info.char_draw_positions[i].y - (str_draw_info.height * ver_align * 0.5f)
        };

        const s_rect_i char_src_rect = font_arrangement_info->chars.src_rects[char_index];

        SubmitToRenderBatch(char_pos, (s_vec_2d) { 0 }, V2(1.0f, 1.0f), 0.0f, font_tex_gl_id, font_tex_size, char_src_rect, blend, draw_phase_state, renderer);
    }
}

void FlushTextureBatch(s_draw_phase_state* const draw_phase_state, const s_renderer* const renderer) {
    if (draw_phase_state->tex_batch_slots_used_cnt == 0) {
        return;
    }

    // Write the batch vertex data to the GPU.
    GL_CALL(glBindVertexArray(renderer->tex_batch_vert_array_gl_id));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, renderer->tex_batch_vert_buf_gl_id));

    const int write_size = TEXTURE_BATCH_SLOT_VERTS_SIZE * draw_phase_state->tex_batch_slots_used_cnt;
    GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, write_size, draw_phase_state->tex_batch_slot_verts));

    // Draw the batch.
    const s_textured_quad_shader_prog* const prog = &renderer->textured_quad_shader_prog;

    GL_CALL(glUseProgram(prog->gl_id));

    GL_CALL(glUniformMatrix4fv(prog->proj_uniform_loc, 1, false, (const float*)&draw_phase_state->proj_mat));
    GL_CALL(glUniformMatrix4fv(prog->view_uniform_loc, 1, false, (const float*)&draw_phase_state->view_mat));

    GL_CALL(glBindTexture(GL_TEXTURE_2D, draw_phase_state->tex_batch_tex_gl_id));

    //GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->tex_batch_elem_buf_gl_id));
    GL_CALL(glDrawElements(GL_TRIANGLES, TEXTURE_BATCH_SLOT_INDICES_CNT * draw_phase_state->tex_batch_slots_used_cnt, GL_UNSIGNED_SHORT, NULL));

    // Clear batch state.
    Clear(draw_phase_state->tex_batch_slot_verts, sizeof(draw_phase_state->tex_batch_slot_verts));
    draw_phase_state->tex_batch_slots_used_cnt = 0;
    draw_phase_state->tex_batch_tex_gl_id = 0;
}

void SetRenderSurface(const int surf_index, s_draw_phase_state* const draw_phase_state, const s_renderer* const renderer) {
    assert(surf_index >= 0 && surf_index < renderer->surfs.cnt);
    assert(draw_phase_state);
    assert(renderer);

#if _DEBUG
    for (int i = 0; i < draw_phase_state->surf_index_stack_height; ++i) {
        if (draw_phase_state->surf_index_stack[i] == surf_index) {
            assert(false && "Attempting to set a render surface that has already been set!");
        }
    }
#endif

    // Add the surface index to the stack.
    assert(draw_phase_state->surf_index_stack_height < RENDER_SURFACE_LIMIT); // NOTE: Maybe replace with proper error?
    draw_phase_state->surf_index_stack[draw_phase_state->surf_index_stack_height] = surf_index;
    ++draw_phase_state->surf_index_stack_height;

    // Bind the surface framebuffer.
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, renderer->surfs.framebuffer_gl_ids[surf_index]));
}

void UnsetRenderSurface(s_draw_phase_state* const draw_phase_state, const s_renderer* const renderer) {
    assert(draw_phase_state);
    assert(draw_phase_state->surf_index_stack_height > 0);
    assert(draw_phase_state->tex_batch_slots_used_cnt == 0); // Make sure that the texture batch has been flushed before unsetting the surface.

    --draw_phase_state->surf_index_stack_height;

    if (draw_phase_state->surf_index_stack_height > 0) {
        const int new_surf_index = draw_phase_state->surf_index_stack[draw_phase_state->surf_index_stack_height - 1];
        const GLuint fb_gl_id = renderer->surfs.framebuffer_gl_ids[new_surf_index];
        GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, fb_gl_id));
    } else {
        GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }
}

void SetRenderSurfaceShaderProg(const int shader_prog_index, s_draw_phase_state* const draw_phase_state) {
    assert(draw_phase_state);
    assert(shader_prog_index >= 0 && shader_prog_index < draw_phase_state->assets->shader_progs.cnt);
    assert(draw_phase_state->tex_batch_slots_used_cnt == 0); // Make sure that the texture batch has been flushed before setting a new surface.
    draw_phase_state->surf_shader_prog_gl_id = draw_phase_state->assets->shader_progs.gl_ids[shader_prog_index];
}

void SetRenderSurfaceShaderProgUniform(const char* const uni_name, const u_shader_uniform_val val, const enum shader_uniform_val_type val_type, s_draw_phase_state* const draw_phase_state) {
    assert(draw_phase_state);
    assert(draw_phase_state->surf_shader_prog_gl_id);

    glUseProgram(draw_phase_state->surf_shader_prog_gl_id);

    const int uni_loc = glGetUniformLocation(draw_phase_state->surf_shader_prog_gl_id, uni_name);
    assert(uni_loc != -1);

    switch (val_type) {
        case ek_shader_uniform_val_type_int:
            GL_CALL(glUniform1i(uni_loc, val.i));
            break;

        case ek_shader_uniform_val_type_float:
            GL_CALL(glUniform1f(uni_loc, val.f));
            break;

        case ek_shader_uniform_val_type_v2:
            GL_CALL(glUniform2fv(uni_loc, 1, (const float*)&val.v2));
            break;

        case ek_shader_uniform_val_type_v3:
            GL_CALL(glUniform3fv(uni_loc, 1, (const float*)&val.v3));
            break;

        case ek_shader_uniform_val_type_v4:
            GL_CALL(glUniform4fv(uni_loc, 1, (const float*)&val.v4));
            break;

        case ek_shader_uniform_val_type_mat4x4:
            GL_CALL(glUniformMatrix4fv(uni_loc, 1, false, (const float*)&val.mat4x4));
            break;

        default:
            assert(false && "Invalid shader uniform value type provided!");
            break;
    }
}

void DrawRenderSurface(const int surf_index, s_draw_phase_state* const draw_phase_state, const s_renderer* const renderer) {
    assert(surf_index >= 0 && surf_index < renderer->surfs.cnt);
    assert(draw_phase_state);
    assert(draw_phase_state->surf_shader_prog_gl_id); // Make sure the surface shader program has been set.
    assert(renderer);

    GL_CALL(glUseProgram(draw_phase_state->surf_shader_prog_gl_id));

    GL_CALL(glActiveTexture(GL_TEXTURE0));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, renderer->surfs.framebuffer_tex_gl_ids[surf_index]));

    GL_CALL(glBindVertexArray(renderer->surf_vert_array_gl_id));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->surf_elem_buf_gl_id));
    GL_CALL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL));

    // Reset the surface shader program. NOTE: Not sure if this should be kept?
    draw_phase_state->surf_shader_prog_gl_id = 0;
}
