#include <zf4_graphics.h>

#include <limits>
#include <zf4_io.h>
#include <stb_image.h>
#include <stb_truetype.h>

namespace zf4::graphics {
    static constexpr s_vec_2d_i g_texture_size_limit = {2048, 2048};
    static constexpr int g_texture_channel_cnt = 4;

    static constexpr int g_textured_quad_shader_prog_vert_cnt = 13;

    static constexpr int g_batch_slot_vert_cnt = g_textured_quad_shader_prog_vert_cnt * 4;
    static constexpr int g_batch_slot_verts_size = sizeof(float) * g_batch_slot_vert_cnt;
    static constexpr int g_batch_slot_indices_cnt = 6;
    static constexpr int g_batch_slot_limit = 2048;

    static bool AttachFramebufferTexture(const GLuint fb_gl_id, const GLuint tex_gl_id, const s_vec_2d_i tex_size) {
        assert(tex_gl_id);
        assert(fb_gl_id);
        assert(tex_size.x > 0 && tex_size.y > 0);

        GL_CALL(glBindTexture(GL_TEXTURE_2D, tex_gl_id));
        GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_size.x, tex_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

        GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, fb_gl_id));

        GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_gl_id, 0));

        const bool success = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

        GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));

        return success;
    }

    static GLuint CreateShaderFromSrc(const char* const src, const bool frag) {
        const GLuint gl_id = GL_CALL(glCreateShader(frag ? GL_FRAGMENT_SHADER : GL_VERTEX_SHADER));
        GL_CALL(glShaderSource(gl_id, 1, &src, nullptr));
        GL_CALL(glCompileShader(gl_id));

        GLint compile_success;
        GL_CALL(glGetShaderiv(gl_id, GL_COMPILE_STATUS, &compile_success));

        if (!compile_success) {
            GL_CALL(glDeleteShader(gl_id));
            return 0;
        }

        return gl_id;
    }

    static GLuint CreateShaderProgFromSrcs(const char* const vert_shader_src, const char* const frag_shader_src) {
        const GLuint vert_shader_gl_id = CreateShaderFromSrc(vert_shader_src, false);

        if (!vert_shader_gl_id) {
            return 0;
        }

        const GLuint frag_shader_gl_id = CreateShaderFromSrc(frag_shader_src, true);

        if (!frag_shader_gl_id) {
            GL_CALL(glDeleteShader(vert_shader_gl_id));
            return 0;
        }

        const GLuint prog_gl_id = GL_CALL(glCreateProgram());
        GL_CALL(glAttachShader(prog_gl_id, vert_shader_gl_id));
        GL_CALL(glAttachShader(prog_gl_id, frag_shader_gl_id));
        GL_CALL(glLinkProgram(prog_gl_id));

        // We no longer need the shaders, as they are now part of the program.
        GL_CALL(glDeleteShader(vert_shader_gl_id));
        GL_CALL(glDeleteShader(frag_shader_gl_id));

        return prog_gl_id;
    }

    static s_textured_quad_shader_prog LoadTexturedQuadShaderProg() {
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

        s_textured_quad_shader_prog prog = {
            .gl_id = CreateShaderProgFromSrcs(vert_shader_src, frag_shader_src)
        };

        assert(prog.gl_id);

        prog.proj_uniform_loc = GL_CALL(glGetUniformLocation(prog.gl_id, "u_proj"));
        prog.view_uniform_loc = GL_CALL(glGetUniformLocation(prog.gl_id, "u_view"));
        prog.textures_uniform_loc = GL_CALL(glGetUniformLocation(prog.gl_id, "u_textures"));

        return prog;
    }

    static void WriteVerts(const s_array<float> verts, const a_gl_id tex_gl_id, const s_rect_edges tex_coords, const s_vec_2d pos, const s_vec_2d size, const s_vec_2d origin, const float rot, const s_vec_4d blend) {
        assert(verts.len == g_textured_quad_shader_prog_vert_cnt * 4);

        verts[0] = 0.0f - origin.x;
        verts[1] = 0.0f - origin.y;
        verts[2] = pos.x;
        verts[3] = pos.y;
        verts[4] = size.x;
        verts[5] = size.y;
        verts[6] = rot;
        verts[7] = tex_coords.left;
        verts[8] = tex_coords.top;
        verts[9] = blend.x;
        verts[10] = blend.y;
        verts[11] = blend.z;
        verts[12] = blend.w;

        verts[13] = 1.0f - origin.x;
        verts[14] = 0.0f - origin.y;
        verts[15] = pos.x;
        verts[16] = pos.y;
        verts[17] = size.x;
        verts[18] = size.y;
        verts[19] = rot;
        verts[20] = tex_coords.right;
        verts[21] = tex_coords.top;
        verts[22] = blend.x;
        verts[23] = blend.y;
        verts[24] = blend.z;
        verts[25] = blend.w;

        verts[26] = 1.0f - origin.x;
        verts[27] = 1.0f - origin.y;
        verts[28] = pos.x;
        verts[29] = pos.y;
        verts[30] = size.x;
        verts[31] = size.y;
        verts[32] = rot;
        verts[33] = tex_coords.right;
        verts[34] = tex_coords.bottom;
        verts[35] = blend.x;
        verts[36] = blend.y;
        verts[37] = blend.z;
        verts[38] = blend.w;

        verts[39] = 0.0f - origin.x;
        verts[40] = 1.0f - origin.y;
        verts[41] = pos.x;
        verts[42] = pos.y;
        verts[43] = size.x;
        verts[44] = size.y;
        verts[45] = rot;
        verts[46] = tex_coords.left;
        verts[47] = tex_coords.bottom;
        verts[48] = blend.x;
        verts[49] = blend.y;
        verts[50] = blend.z;
        verts[51] = blend.w;
    }

    static s_rect_edges CalcTexCoords(const s_rect_i src_rect, const s_vec_2d_i tex_size) {
        return {
            static_cast<float>(src_rect.x) / tex_size.x,
            static_cast<float>(src_rect.y) / tex_size.y,
            static_cast<float>(RectRight(src_rect)) / tex_size.x,
            static_cast<float>(RectBottom(src_rect)) / tex_size.y
        };
    }

    static s_batch GenBatch() {
        s_batch batch = {};

        // Generate vertex array.
        GL_CALL(glGenVertexArrays(1, &batch.vert_array_gl_id));
        GL_CALL(glBindVertexArray(batch.vert_array_gl_id));

        // Generate vertex buffer.
        GL_CALL(glGenBuffers(1, &batch.vert_buf_gl_id));
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, batch.vert_buf_gl_id));
        GL_CALL(glBufferData(GL_ARRAY_BUFFER, g_batch_slot_verts_size * g_batch_slot_limit, nullptr, GL_DYNAMIC_DRAW));

        // Generate element buffer.
        GL_CALL(glGenBuffers(1, &batch.elem_buf_gl_id));
        GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch.elem_buf_gl_id));

        static unsigned short indices[6 * g_batch_slot_limit]; // TEMP!

        for (int i = 0; i < g_batch_slot_limit; i++) {
            indices[(i * 6) + 0] = static_cast<unsigned short>((i * 4) + 0);
            indices[(i * 6) + 1] = static_cast<unsigned short>((i * 4) + 1);
            indices[(i * 6) + 2] = static_cast<unsigned short>((i * 4) + 2);
            indices[(i * 6) + 3] = static_cast<unsigned short>((i * 4) + 2);
            indices[(i * 6) + 4] = static_cast<unsigned short>((i * 4) + 3);
            indices[(i * 6) + 5] = static_cast<unsigned short>((i * 4) + 0);
        }

        GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));
        //GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, ArraySizeInBytes(static_cast<s_array<const unsigned short>>(indices)), indices.elems_raw, GL_STATIC_DRAW));

        // Set vertex attribute pointers.
        const int verts_stride = sizeof(float) * g_textured_quad_shader_prog_vert_cnt;

        GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, false, verts_stride, reinterpret_cast<const void*>(sizeof(float) * 0)));
        GL_CALL(glEnableVertexAttribArray(0));

        GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, false, verts_stride, reinterpret_cast<const void*>(sizeof(float) * 2)));
        GL_CALL(glEnableVertexAttribArray(1));

        GL_CALL(glVertexAttribPointer(2, 2, GL_FLOAT, false, verts_stride, reinterpret_cast<const void*>(sizeof(float) * 4)));
        GL_CALL(glEnableVertexAttribArray(2));

        GL_CALL(glVertexAttribPointer(3, 1, GL_FLOAT, false, verts_stride, reinterpret_cast<const void*>(sizeof(float) * 6)));
        GL_CALL(glEnableVertexAttribArray(3));

        GL_CALL(glVertexAttribPointer(4, 2, GL_FLOAT, false, verts_stride, reinterpret_cast<const void*>(sizeof(float) * 7)));
        GL_CALL(glEnableVertexAttribArray(4));

        GL_CALL(glVertexAttribPointer(5, 4, GL_FLOAT, false, verts_stride, reinterpret_cast<const void*>(sizeof(float) * 9)));
        GL_CALL(glEnableVertexAttribArray(5));

        return batch;
    }

    s_str_draw_info GenStrDrawInfo(const char* const str, const int font_index, const s_fonts& fonts, const s_vec_2d pos, const e_str_hor_align hor_align, const e_str_ver_align ver_align) {
        assert(str && str[0]);

        s_str_draw_info draw_info = {
            .line_infos = {.len = 1}
        };

        //
        // First Pass
        //
        const s_font_arrangement_info& font_arrangement_info = fonts.arrangement_infos[font_index];

        s_vec_2d chr_draw_pos_pen = {};

        do {
            ++draw_info.chr_draw_rects.len;

            const char chr = str[draw_info.chr_draw_rects.len - 1];

            if (chr != '\n') {
                const int chr_index = chr - g_font_char_range_begin;

                const s_vec_2d chr_draw_pos = {
                    chr_draw_pos_pen.x + font_arrangement_info.chrs.hor_offsets[chr_index],
                    chr_draw_pos_pen.y + font_arrangement_info.chrs.ver_offsets[chr_index]
                };

                ListEnd(draw_info.chr_draw_rects) = GenRect(chr_draw_pos, RectSize(font_arrangement_info.chrs.src_rects[chr_index]));

                chr_draw_pos_pen.x += font_arrangement_info.chrs.hor_advances[chr_index];
            } else {
                ListEnd(draw_info.line_infos).width_including_offs = chr_draw_pos_pen.x;
                ++draw_info.line_infos.len;
                ListEnd(draw_info.line_infos).begin_chr_index = draw_info.chr_draw_rects.len;

                chr_draw_pos_pen.x = 0;
                chr_draw_pos_pen.y += font_arrangement_info.line_height;
            }
        } while (str[draw_info.chr_draw_rects.len]);

        ListEnd(draw_info.line_infos).width_including_offs = chr_draw_pos_pen.x;
        const float height_including_offs = chr_draw_pos_pen.y + font_arrangement_info.line_height;

        //
        // Second Pass: Applying Position and Alignment Offsets
        //
        const float ver_align_offs = -(height_including_offs * (static_cast<float>(ver_align) / 2.0f));

        for (int i = 0; i < draw_info.line_infos.len; i++) {
            const int line_end_chr_index = i < draw_info.line_infos.len - 1 ? draw_info.line_infos[i + 1].begin_chr_index - 1 : draw_info.chr_draw_rects.len - 1;

            const float hor_align_offs = -(draw_info.line_infos[i].width_including_offs * (static_cast<float>(hor_align) / 2.0f));

            const s_vec_2d offs = pos + s_vec_2d(hor_align_offs, ver_align_offs);

            for (int j = draw_info.line_infos[i].begin_chr_index; j <= line_end_chr_index; ++j) {
                draw_info.chr_draw_rects[j] = RectTranslated(draw_info.chr_draw_rects[j], offs);
            }
        }

        return draw_info;
    }

    s_rect GenStrCollider(const s_str_draw_info& draw_info) {
        const int str_len = draw_info.chr_draw_rects.len;
        const int line_cnt = draw_info.line_infos.len;

        // Get top and bottom.
        float top = std::numeric_limits<float>().max();
        float bottom = std::numeric_limits<float>().lowest();

        if (line_cnt == 1) {
            for (int i = 0; i < str_len; ++i) {
                top = Min(top, draw_info.chr_draw_rects[i].y);
                bottom = Max(bottom, RectBottom(draw_info.chr_draw_rects[i]));
            }
        } else {
            for (int i = 0; i < draw_info.line_infos[1].begin_chr_index; ++i) {
                top = Min(top, draw_info.chr_draw_rects[i].y);
            }

            for (int i = draw_info.line_infos[draw_info.line_infos.len - 1].begin_chr_index; i < str_len; ++i) {
                bottom = Max(bottom, RectBottom(draw_info.chr_draw_rects[i]));
            }
        }

        // Get left and right.
        float left = std::numeric_limits<float>().max();
        float right = std::numeric_limits<float>().lowest();

        for (int i = 0; i < line_cnt; ++i) {
            const int begin_chr_index = draw_info.line_infos[i].begin_chr_index;
            left = Min(left, draw_info.chr_draw_rects[begin_chr_index].x);

            const int end_chr_index = i < line_cnt - 1 ? draw_info.line_infos[i + 1].begin_chr_index : str_len - 1;
            right = Max(right, RectRight(draw_info.chr_draw_rects[end_chr_index]));
        }

        return {left, top, right - left, bottom - top};
    }

    s_textures PushTextures(const int cnt, const s_array<const char* const> filenames, s_mem_arena& mem_arena, bool& err) {
        assert(cnt > 0);
        assert(cnt == filenames.len);
        assert(!err);

        const auto gl_ids = PushArray<a_gl_id>(cnt, mem_arena);

        if (IsStructZero(gl_ids)) {
            err = true;
            return {};
        }

        const auto sizes = PushArray<s_vec_2d_i>(cnt, mem_arena);

        if (IsStructZero(sizes)) {
            err = true;
            return {};
        }

        glGenTextures(cnt, gl_ids.elems_raw);

        for (int i = 0; i < cnt; ++i) {
            stbi_uc* const px_data = stbi_load(filenames[i], &sizes[i].x, &sizes[i].y, nullptr, 4);

            if (!px_data) {
                LogError("Failed to load texture with filename \"%s\"!", filenames[i]);
                err = true;
                break;
            }

            GL_CALL(glBindTexture(GL_TEXTURE_2D, gl_ids[i]));
            GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
            GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
            GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sizes[i].x, sizes[i].y, 0, GL_RGBA, GL_UNSIGNED_BYTE, px_data));

            stbi_image_free(px_data);
        }

        if (err) {
            glDeleteTextures(cnt, gl_ids.elems_raw);
            return {};
        }

        return {
            .gl_ids = gl_ids,
            .sizes = sizes,
            .cnt = cnt
        };
    }

    void UnloadTextures(const s_textures& textures) {
        glDeleteTextures(textures.gl_ids.len, textures.gl_ids.elems_raw);
    }

    s_fonts PushFonts(const int cnt, const s_array<const char* const> filenames, const s_array<const int> heights, s_mem_arena& mem_arena, s_mem_arena& scratch_space, bool& err) {
        assert(cnt > 0);
        assert(cnt == filenames.len);
        assert(cnt == heights.len);
        assert(!err);

        // Reserve memory for font data.
        const auto arrangement_infos = PushArray<s_font_arrangement_info>(cnt, mem_arena);

        if (IsStructZero(arrangement_infos)) {
            err = true;
            return {};
        }

        const auto tex_gl_ids = PushArray<a_gl_id>(cnt, mem_arena);

        if (IsStructZero(tex_gl_ids)) {
            err = true;
            return {};
        }

        const auto tex_sizes = PushArray<s_vec_2d_i>(cnt, mem_arena);

        if (IsStructZero(tex_sizes)) {
            err = true;
            return {};
        }

        // Reserve scratch space for texture pixel data, to be reused for all font atlases.
        const auto px_data_scratch_space = PushArray<a_byte>(g_texture_size_limit.x * g_texture_size_limit.y, scratch_space);

        if (IsStructZero(px_data_scratch_space)) {
            err = true;
            return {};
        }

        // Generate all font textures upfront.
        glGenTextures(cnt, tex_gl_ids.elems_raw);

        // Do an iteration for each font...
        for (int i = 0; i < cnt; ++i) {
            const int scratch_space_init_offs = scratch_space.offs;

            // Get the contents of the font file.
            const auto file_contents = PushFileContents(filenames[i], scratch_space, err);

            if (err) {
                LogError("Failed to load file contents of font \"%s\".", filenames[i]);
                break;
            }

            // Initialise the font.
            stbtt_fontinfo font;

            err = !stbtt_InitFont(&font, file_contents.elems_raw, stbtt_GetFontOffsetForIndex(file_contents.elems_raw, 0));

            if (err) {
                LogError("Failed to initialize font \"%s\".", filenames[i]);
                break;
            }

            // Extract basic font metrics and calculate line height.
            const float scale = stbtt_ScaleForPixelHeight(&font, static_cast<float>(heights[i]));

            int ascent, descent, line_gap;
            stbtt_GetFontVMetrics(&font, &ascent, &descent, &line_gap);

            arrangement_infos[i].line_height = static_cast<int>((ascent - descent + line_gap) * scale);

            // Set the pixel data for the font atlas texture.
            ZeroOutArrayElems(px_data_scratch_space);

            s_vec_2d_i char_draw_pos = {};

            for (int j = 0; j < g_font_char_range_len; j++) {
                const char chr = g_font_char_range_begin + j;

                int advance;
                stbtt_GetCodepointHMetrics(&font, chr, &advance, nullptr);

                arrangement_infos[i].chrs.hor_advances[j] = static_cast<int>(advance * scale);

                if (chr == ' ') {
                    continue;
                }

                s_vec_2d_i size, offs;
                unsigned char* const bitmap_raw = stbtt_GetCodepointBitmap(&font, 0, scale, chr, &size.x, &size.y, &offs.x, &offs.y);

                err = !bitmap_raw;

                if (err) {
                    LogError("Failed to get a character glyph bitmap from \"%s\" for character '%c'!", filenames[i], chr);
                    break;
                }

                const s_array<const a_byte> bitmap = {
                    .elems_raw = bitmap_raw,
                    .len = size.x * size.y
                };

                if (char_draw_pos.x + size.x > g_texture_size_limit.x) {
                    char_draw_pos.x = 0;
                    char_draw_pos.y += arrangement_infos[i].line_height;
                }

                arrangement_infos[i].chrs.hor_offsets[j] = offs.x;
                arrangement_infos[i].chrs.ver_offsets[j] = offs.y + arrangement_infos[i].line_height;
                arrangement_infos[i].chrs.src_rects[j] = GenRect(char_draw_pos, size);

                for (int y = 0; y < size.y; y++) {
                    for (int x = 0; x < size.x; x++) {
                        const int px_index = (((char_draw_pos.y + y) * g_texture_size_limit.x) + (char_draw_pos.x + x)) * g_texture_channel_cnt;
                        const int bitmap_index = (y * size.x) + x;

                        px_data_scratch_space[px_index + 0] = 255;
                        px_data_scratch_space[px_index + 1] = 255;
                        px_data_scratch_space[px_index + 2] = 255;
                        px_data_scratch_space[px_index + 3] = bitmap[bitmap_index];
                    }
                }

                stbtt_FreeBitmap(bitmap_raw, nullptr);

                char_draw_pos.x += size.x;
            }

            if (err) {
                break;
            }

            RewindMemArena(scratch_space, scratch_space_init_offs);

            GL_CALL(glBindTexture(GL_TEXTURE_2D, tex_gl_ids[i]));
            GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
            GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

            // TODO: Only use the used section of this.
            GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, g_texture_size_limit.x, g_texture_size_limit.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, px_data_scratch_space.elems_raw));

            tex_sizes[i] = g_texture_size_limit; // TEMP
        }

        if (err) {
            glDeleteTextures(tex_gl_ids.len, tex_gl_ids.elems_raw);
            return {};
        }

        return {
            .arrangement_infos = arrangement_infos,
            .tex_gl_ids = tex_gl_ids,
            .tex_sizes = tex_sizes,
            .cnt = cnt
        };
    }

    void UnloadFonts(const s_fonts& fonts) {
        glDeleteTextures(fonts.tex_gl_ids.len, fonts.tex_gl_ids.elems_raw);
    }

    static s_surf_pers_data GenSurfPersData() {
        s_surf_pers_data surf_pers_data = {};

        GL_CALL(glGenVertexArrays(1, &surf_pers_data.vert_array_gl_id));
        GL_CALL(glBindVertexArray(surf_pers_data.vert_array_gl_id));

        GL_CALL(glGenBuffers(1, &surf_pers_data.vert_buf_gl_id));
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, surf_pers_data.vert_buf_gl_id));

        {
            const float verts[] = {
                -1.0f, -1.0f, 0.0f, 0.0f,
                1.0f, -1.0f, 1.0f, 0.0f,
                1.0f, 1.0f, 1.0f, 1.0f,
                -1.0f, 1.0f, 0.0f, 1.0f
            };

            GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW));
        }

        GL_CALL(glGenBuffers(1, &surf_pers_data.elem_buf_gl_id));
        GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, surf_pers_data.elem_buf_gl_id));

        {
            const unsigned short indices[] = {0, 1, 2, 2, 3, 0};
            GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));
        }

        GL_CALL(glEnableVertexAttribArray(0));
        GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(float) * 4, (const void*)(sizeof(float) * 0)));

        GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(float) * 4, (const void*)(sizeof(float) * 2)));
        GL_CALL(glEnableVertexAttribArray(1));

        GL_CALL(glBindVertexArray(0));

        return surf_pers_data;
    }

    s_pers_render_data GenPersRenderData() {
        return {
            .textured_quad_shader_prog = LoadTexturedQuadShaderProg(),
            .batch = GenBatch(),
            .surf_pers_data = GenSurfPersData()
        };
    }

    void CleanPersRenderData(const s_pers_render_data& render_data) {
        GL_CALL(glDeleteBuffers(1, &render_data.batch.elem_buf_gl_id));
        GL_CALL(glDeleteBuffers(1, &render_data.batch.vert_buf_gl_id));
        GL_CALL(glDeleteVertexArrays(1, &render_data.batch.vert_array_gl_id));

        GL_CALL(glDeleteBuffers(1, &render_data.surf_pers_data.elem_buf_gl_id));
        GL_CALL(glDeleteBuffers(1, &render_data.surf_pers_data.vert_buf_gl_id));
        GL_CALL(glDeleteVertexArrays(1, &render_data.surf_pers_data.vert_array_gl_id));
    }

    bool InitSurfaces(const int cnt, s_surfaces& surfs, const s_vec_2d_i window_size) {
        assert(cnt > 0 && cnt <= g_surface_limit);
        assert(IsStructZero(surfs));
        assert(window_size.x > 0 && window_size.y > 0);

        surfs.cnt = cnt;

        GL_CALL(glGenFramebuffers(cnt, surfs.framebuffer_gl_ids.elems_raw));
        GL_CALL(glGenTextures(cnt, surfs.framebuffer_tex_gl_ids.elems_raw));

        for (int i = 0; i < cnt; ++i) {
            if (!AttachFramebufferTexture(surfs.framebuffer_gl_ids[i], surfs.framebuffer_tex_gl_ids[i], window_size)) {
                return false;
            }
        }

        return true;
    }

    void CleanAndZeroOutSurfaces(s_surfaces& surfs) {
        GL_CALL(glDeleteTextures(surfs.cnt, surfs.framebuffer_tex_gl_ids.elems_raw));
        GL_CALL(glDeleteFramebuffers(surfs.cnt, surfs.framebuffer_gl_ids.elems_raw));

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

    bool ExecDrawInstrs(const s_array<const s_draw_instr> instrs, const s_vec_2d_i window_size, const s_pers_render_data& pers_render_data, const s_surfaces& surfs, const s_textures& textures, s_mem_arena& scratch_space) {
        const auto batch_verts = PushArray<float>(g_batch_slot_vert_cnt * g_batch_slot_limit, scratch_space);

        if (IsStructZero(batch_verts)) {
            return false;
        }

        int batch_slots_used_cnt = 0;
        a_gl_id batch_tex_gl_id = 0; // NOTE: In the future we will support multiple texture units. This is just for simplicity right now.

        a_gl_id surf_shader_prog_gl_id = 0; // When we draw a surface, it will use this shader program.
        s_static_list<int, g_surface_limit> surf_index_stack = {};

        const s_matrix_4x4 proj_mat = GenOrthoMatrix4x4(0.0f, window_size.x, window_size.y, 0.0f, -1.0f, 1.0f);
        s_matrix_4x4 view_mat = {};

        // NOTE: The blend setup could be moved into game initialisation.
        GL_CALL(glEnable(GL_BLEND));
        GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        for (int i = 0; i < instrs.len; ++i) {
            //
            // Instruction Processing
            //
            const s_draw_instr& instr = instrs[i];

            switch (instr.type) {
                case ek_instr_type_clear:
                    glClearColor(instr.clear.color.x, instr.clear.color.y, instr.clear.color.z, instr.clear.color.w);
                    glClear(GL_COLOR_BUFFER_BIT);
                    break;

                case ek_instr_type_set_view_matrix:
                    view_mat = instr.set_view_mat.mat;
                    break;

                case ek_instr_type_draw_texture:
                    {
                        static_assert(g_batch_slot_limit > 0);

                        if (batch_slots_used_cnt == 0) {
                            batch_tex_gl_id = instr.draw_tex.tex_gl_id;
                        }

                        const s_array<float> slot_verts = {
                            .elems_raw = batch_verts.elems_raw + (g_batch_slot_vert_cnt * batch_slots_used_cnt),
                            .len = g_batch_slot_vert_cnt
                        };

                        WriteVerts(slot_verts, instr.draw_tex.tex_gl_id, instr.draw_tex.tex_coords, instr.draw_tex.pos, instr.draw_tex.size, instr.draw_tex.origin, instr.draw_tex.rot, instr.draw_tex.blend);

                        ++batch_slots_used_cnt;
                    }

                    break;

                case ek_instr_type_set_surf:
                    assert(instr.set_surf.surf_index >= 0 && instr.set_surf.surf_index < surfs.cnt);

                    // Add the surface index to the stack.
                    ListAppend(surf_index_stack, instr.set_surf.surf_index);

                    // Bind the surface framebuffer.
                    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, surfs.framebuffer_gl_ids[instr.set_surf.surf_index]));

                    break;

                case ek_instr_type_unset_surf:
                    ListPop(surf_index_stack);

                    if (IsListEmpty(surf_index_stack)) {
                        GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
                    } else {
                        const int new_surf_index = ListEnd(surf_index_stack);
                        const GLuint fb_gl_id = surfs.framebuffer_gl_ids[new_surf_index];
                        GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, surfs.framebuffer_gl_ids[new_surf_index]));
                    }

                    break;

                case ek_instr_type_draw_surf:
                    assert(instr.draw_surf.surf_index >= 0 && instr.draw_surf.surf_index < surfs.cnt);
                    assert(surf_shader_prog_gl_id); // Make sure the surface shader program has been set.

                    GL_CALL(glUseProgram(surf_shader_prog_gl_id));

                    GL_CALL(glActiveTexture(GL_TEXTURE0));
                    GL_CALL(glBindTexture(GL_TEXTURE_2D, surfs.framebuffer_tex_gl_ids[instr.draw_surf.surf_index]));

                    GL_CALL(glBindVertexArray(pers_render_data.surf_pers_data.vert_array_gl_id));
                    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pers_render_data.surf_pers_data.elem_buf_gl_id));
                    GL_CALL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr));

                    break;

                case ek_instr_type_set_surf_shader_prog_uniform:
                    assert(surf_shader_prog_gl_id); // Make sure the surface shader program has been set.

                    glUseProgram(surf_shader_prog_gl_id);

                    const int uni_loc = glGetUniformLocation(surf_shader_prog_gl_id, instr.set_surf_shader_prog_uni.name);
                    assert(uni_loc != -1);

                    switch (instr.set_surf_shader_prog_uni.val_type) {
                        case ek_shader_uniform_val_type_int:
                            GL_CALL(glUniform1i(uni_loc, instr.set_surf_shader_prog_uni.val_as_int));
                            break;

                        case ek_shader_uniform_val_type_float:
                            GL_CALL(glUniform1f(uni_loc, instr.set_surf_shader_prog_uni.val_as_float));
                            break;

                        case ek_shader_uniform_val_type_v2:
                            GL_CALL(glUniform2fv(uni_loc, 1, reinterpret_cast<const float*>(&instr.set_surf_shader_prog_uni.val_as_v2)));
                            break;

                        case ek_shader_uniform_val_type_v3:
                            GL_CALL(glUniform3fv(uni_loc, 1, reinterpret_cast<const float*>(&instr.set_surf_shader_prog_uni.val_as_v3)));
                            break;

                        case ek_shader_uniform_val_type_v4:
                            GL_CALL(glUniform4fv(uni_loc, 1, reinterpret_cast<const float*>(&instr.set_surf_shader_prog_uni.val_as_v4)));
                            break;

                        case ek_shader_uniform_val_type_mat_4x4:
                            GL_CALL(glUniformMatrix4fv(uni_loc, 1, false, reinterpret_cast<const float*>(&instr.set_surf_shader_prog_uni.val_as_mat_4x4)));
                            break;
                    }

                    break;
            }

            //
            // Batch Flushing
            //
            if (batch_slots_used_cnt == 0) {
                // There is no case where we need to flush and no slots in the current batch have been used.
                continue;
            }

            bool flush = false;

            if (i == instrs.len - 1) {
                flush = true;
            } else {
                const s_draw_instr& instr_next = instrs[i + 1];

                if (batch_slots_used_cnt == g_batch_slot_limit
                    || instr_next.type == ek_instr_type_set_surf
                    || instr_next.type == ek_instr_type_unset_surf
                    || (instr_next.type == ek_instr_type_draw_texture && instr_next.draw_tex.tex_gl_id != batch_tex_gl_id)) {
                    flush = true;
                }
            }

            if (flush) {
                // Write the batch vertex data to the GPU.
                GL_CALL(glBindVertexArray(pers_render_data.batch.vert_array_gl_id));
                GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, pers_render_data.batch.vert_buf_gl_id));

                const int write_size = g_batch_slot_verts_size * batch_slots_used_cnt;
                GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, write_size, batch_verts.elems_raw));

                // Draw the batch.
                const s_textured_quad_shader_prog& prog = pers_render_data.textured_quad_shader_prog;

                GL_CALL(glUseProgram(prog.gl_id));

                GL_CALL(glUniformMatrix4fv(prog.proj_uniform_loc, 1, false, reinterpret_cast<const float*>(&proj_mat)));
                GL_CALL(glUniformMatrix4fv(prog.view_uniform_loc, 1, false, reinterpret_cast<const float*>(&view_mat)));

                GL_CALL(glBindTexture(GL_TEXTURE_2D, batch_tex_gl_id));

                GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pers_render_data.batch.elem_buf_gl_id));
                GL_CALL(glDrawElements(GL_TRIANGLES, g_batch_slot_indices_cnt * batch_slots_used_cnt, GL_UNSIGNED_SHORT, nullptr));

                // Clear batch state.
                ZeroOutArrayElems(batch_verts);
                batch_slots_used_cnt = 0;
                batch_tex_gl_id = 0;
            }
        }

        return true;
    }

    bool AppendDrawTextureInstr(s_list<s_draw_instr>& instrs, const int tex_index, const s_textures& textures, const s_rect_i src_rect, const s_vec_2d pos, const s_vec_2d origin, const s_vec_2d scale, const float rot, const s_vec_4d blend) {
        const s_vec_2d_i tex_size = textures.sizes[tex_index];

        const s_draw_texture_instr draw_tex = {
            .tex_gl_id = textures.gl_ids[tex_index],
            .tex_coords = CalcTexCoords(src_rect, tex_size),
            .pos = pos,
            .size = {src_rect.width * scale.x, src_rect.height * scale.y},
            .origin = origin,
            .rot = rot,
            .blend = blend
        };

        return zf4::ListAppendTry(instrs, {
            .type = ek_instr_type_draw_texture,
            .draw_tex = draw_tex
        });
    }

    bool AppendDrawTextureInstrsForStr(s_list<s_draw_instr>& instrs, const char* const str, const int font_index, const s_fonts& fonts, const s_vec_2d pos, const s_vec_4d blend, const e_str_hor_align hor_align, const e_str_ver_align ver_align) {
        assert(str);
        assert(font_index >= 0 && font_index < fonts.cnt);

        const s_str_draw_info str_draw_info = GenStrDrawInfo(str, font_index, fonts, pos, hor_align, ver_align); // TODO: Cache this, but only if it's actually a bottleneck on performance.

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
            const s_rect_i char_src_rect = font_arrangement_info.chrs.src_rects[char_index];

            const s_draw_texture_instr draw_tex = {
                .tex_gl_id = font_tex_gl_id,
                .tex_coords = CalcTexCoords(char_src_rect, font_tex_size),
                .pos = RectTopLeft(str_draw_info.chr_draw_rects[i]),
                .size = RectSize(str_draw_info.chr_draw_rects[i]),
                .blend = blend
            };

            const s_draw_instr instr = {
                .type = ek_instr_type_draw_texture,
                .draw_tex = draw_tex
            };

            if (!zf4::ListAppendTry(instrs, instr)) {
                return false;
            }
        }

        return true;
    }
}
