#include <zf4_graphics.h>

#include <limits>
#include <zf4_io.h>
#include <stb_image.h>
#include <stb_truetype.h>

#define GL_CALL(X) X; assert(glGetError() == GL_NO_ERROR)

namespace zf4::graphics {
    static constexpr int g_texture_channel_cnt = 4;
    static constexpr size_t g_font_texture_width = 2048;
    static constexpr size_t g_font_texture_height_limit = 2048;

    static constexpr int g_textured_quad_shader_prog_vert_cnt = 13;

    static constexpr int g_batch_slot_vert_cnt = g_textured_quad_shader_prog_vert_cnt * 4;
    static constexpr int g_batch_slot_verts_size = sizeof(float) * g_batch_slot_vert_cnt;
    static constexpr int g_batch_slot_indices_cnt = 6;
    static constexpr int g_batch_slot_limit = 2048;

    static s_textures PushTextures(const size_t tex_cnt, const a_tex_index_to_file_path_mapper tex_index_to_file_path, s_mem_arena& mem_arena) {
        assert(tex_cnt > 0);
        assert(tex_index_to_file_path);
        assert(mem_arena.IsInitialized());

        const auto gl_ids = PushArray<a_gl_id>(tex_cnt, mem_arena);

        if (gl_ids.IsInitialized()) {
            return {};
        }

        const auto sizes = PushArray<s_vec_2d_i>(tex_cnt, mem_arena);

        if (sizes.IsInitialized()) {
            return {};
        }

        glGenTextures(gl_ids.len, gl_ids.elems_raw);

        bool err = false;

        for (int i = 0; i < tex_cnt; ++i) {
            const auto fp = tex_index_to_file_path(i);
            assert(fp);

            stbi_uc* const px_data = stbi_load(fp, &sizes[i].x, &sizes[i].y, nullptr, 4);

            if (!px_data) {
                LogError("Failed to load texture with file path \"%s\"!", fp);
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
            glDeleteTextures(gl_ids.len, gl_ids.elems_raw);
            return {};
        }

        return {
            .gl_ids = gl_ids,
            .sizes = sizes,
            .cnt = tex_cnt
        };
    }

    static s_fonts PushFonts(const size_t font_cnt, const a_font_index_to_info_mapper font_index_to_info, s_mem_arena& mem_arena, s_mem_arena& scratch_space) {
        assert(font_cnt > 0);
        assert(font_index_to_info);
        assert(mem_arena.IsInitialized());
        assert(scratch_space.IsInitialized());

        // Reserve memory for font data.
        const auto arrangement_infos = PushArray<s_font_arrangement_info>(font_cnt, mem_arena);

        if (!arrangement_infos.IsValid()) {
            return {};
        }

        const auto tex_gl_ids = PushArray<a_gl_id>(font_cnt, mem_arena);

        if (!tex_gl_ids.IsValid()) {
            return {};
        }

        const auto tex_heights = PushArray<size_t>(font_cnt, mem_arena);

        if (!tex_heights.IsValid()) {
            return {};
        }

        // Reserve scratch space for texture pixel data, to be reused for all font atlases.
        const auto px_data_scratch_space = PushArray<a_byte>(g_font_texture_width * g_font_texture_height_limit, scratch_space);

        if (!px_data_scratch_space.IsValid()) {
            return {};
        }

        // Generate all font textures upfront.
        glGenTextures(tex_gl_ids.len, tex_gl_ids.elems_raw);

        // Do an iteration for each font...
        bool err = false;

        for (int i = 0; i < font_cnt; ++i) {
            const auto scratch_space_init_offs = scratch_space.offs;

            const auto font_info = font_index_to_info(i);
            assert(font_info.file_path);
            assert(font_info.height > 0);

            // Get the contents of the font file.
            const auto file_contents = PushFileContents(font_info.file_path, scratch_space);

            if (!file_contents.IsValid()) {
                LogError("Failed to load file contents of font \"%s\".", font_info.file_path);
                err = true;
                break;
            }

            // Initialise the font.
            stbtt_fontinfo font;

            if (!stbtt_InitFont(&font, file_contents.elems_raw, stbtt_GetFontOffsetForIndex(file_contents.elems_raw, 0))) {
                LogError("Failed to initialize font \"%s\".", font_info.file_path);
                err = true;
                break;
            }

            // Extract basic font metrics and calculate line height.
            const float scale = stbtt_ScaleForPixelHeight(&font, static_cast<float>(font_info.height));

            int ascent, descent, line_gap;
            stbtt_GetFontVMetrics(&font, &ascent, &descent, &line_gap);

            arrangement_infos[i].line_height = static_cast<int>((ascent - descent + line_gap) * scale);

            // Set the pixel data for the font atlas texture, and record the character metrics.
            memset(px_data_scratch_space.elems_raw, 0, px_data_scratch_space.SizeInBytes());

            s_vec_2d_i char_draw_pos;

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

                if (!bitmap_raw) {
                    LogError("Failed to get a character glyph bitmap from \"%s\" for character '%c'!", font_info.file_path, chr);
                    err = true;
                    break;
                }

                const s_array<const a_byte> bitmap = {bitmap_raw, static_cast<size_t>(size.x) * static_cast<size_t>(size.y)};

                if (char_draw_pos.x + size.x > g_font_texture_width) {
                    char_draw_pos.x = 0;
                    char_draw_pos.y += arrangement_infos[i].line_height;
                }

                tex_heights[i] = Max(char_draw_pos.y + size.y, tex_heights[i]);

                if (tex_heights[i] > g_font_texture_height_limit) {
                    LogError("Font texture height limit exceeded for font \"%s\"!", font_info.file_path);
                    err = true;
                    break;
                }

                arrangement_infos[i].chrs.hor_offsets[j] = offs.x;
                arrangement_infos[i].chrs.ver_offsets[j] = offs.y + (ascent * scale);
                arrangement_infos[i].chrs.src_rects[j] = {char_draw_pos, size};

                for (int y = 0; y < size.y; y++) {
                    for (int x = 0; x < size.x; x++) {
                        const int px_index = (((char_draw_pos.y + y) * g_font_texture_width) + (char_draw_pos.x + x)) * g_texture_channel_cnt;
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

            scratch_space.offs = scratch_space_init_offs;

            GL_CALL(glBindTexture(GL_TEXTURE_2D, tex_gl_ids[i]));
            GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
            GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
            GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, g_font_texture_width, tex_heights[i], 0, GL_RGBA, GL_UNSIGNED_BYTE, px_data_scratch_space.elems_raw));
        }

        if (err) {
            glDeleteTextures(tex_gl_ids.len, tex_gl_ids.elems_raw);
            return {};
        }

        return {
            .arrangement_infos = arrangement_infos,
            .tex_gl_ids = tex_gl_ids,
            .tex_heights = tex_heights,
            .cnt = font_cnt
        };
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
        assert(verts.IsValid());
        assert(verts.len == g_textured_quad_shader_prog_vert_cnt * 4);
        assert(tex_gl_id);

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
            static_cast<float>(src_rect.Right()) / tex_size.x,
            static_cast<float>(src_rect.Bottom()) / tex_size.y
        };
    }

    static s_gl_ids GenBatchGLBufs() {
        s_gl_ids gl_ids = {};

        // Generate vertex array.
        GL_CALL(glGenVertexArrays(1, &gl_ids.vert_array_gl_id));
        GL_CALL(glBindVertexArray(gl_ids.vert_array_gl_id));

        // Generate vertex buffer.
        GL_CALL(glGenBuffers(1, &gl_ids.vert_buf_gl_id));
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, gl_ids.vert_buf_gl_id));
        GL_CALL(glBufferData(GL_ARRAY_BUFFER, g_batch_slot_verts_size * g_batch_slot_limit, nullptr, GL_DYNAMIC_DRAW));

        // Generate element buffer.
        GL_CALL(glGenBuffers(1, &gl_ids.elem_buf_gl_id));
        GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_ids.elem_buf_gl_id));

        s_static_array<unsigned short, 6 * g_batch_slot_limit> indices;

        for (int i = 0; i < g_batch_slot_limit; i++) {
            indices[(i * 6) + 0] = static_cast<unsigned short>((i * 4) + 0);
            indices[(i * 6) + 1] = static_cast<unsigned short>((i * 4) + 1);
            indices[(i * 6) + 2] = static_cast<unsigned short>((i * 4) + 2);
            indices[(i * 6) + 3] = static_cast<unsigned short>((i * 4) + 2);
            indices[(i * 6) + 4] = static_cast<unsigned short>((i * 4) + 3);
            indices[(i * 6) + 5] = static_cast<unsigned short>((i * 4) + 0);
        }

        GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.elems_raw, GL_STATIC_DRAW));

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

        return gl_ids;
    }

    static s_gl_ids GenSurfGLBufs() {
        s_gl_ids gl_ids = {};

        GL_CALL(glGenVertexArrays(1, &gl_ids.vert_array_gl_id));
        GL_CALL(glBindVertexArray(gl_ids.vert_array_gl_id));

        GL_CALL(glGenBuffers(1, &gl_ids.vert_buf_gl_id));
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, gl_ids.vert_buf_gl_id));

        {
            const float verts[] = {
                -1.0f, -1.0f, 0.0f, 0.0f,
                1.0f, -1.0f, 1.0f, 0.0f,
                1.0f, 1.0f, 1.0f, 1.0f,
                -1.0f, 1.0f, 0.0f, 1.0f
            };

            GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW));
        }

        GL_CALL(glGenBuffers(1, &gl_ids.elem_buf_gl_id));
        GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_ids.elem_buf_gl_id));

        {
            const unsigned short indices[] = {0, 1, 2, 2, 3, 0};
            GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));
        }

        GL_CALL(glEnableVertexAttribArray(0));
        GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(float) * 4, (const void*)(sizeof(float) * 0)));

        GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(float) * 4, (const void*)(sizeof(float) * 2)));
        GL_CALL(glEnableVertexAttribArray(1));

        GL_CALL(glBindVertexArray(0));

        return gl_ids;
    }

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

    bool s_pers_render_data::Init(const size_t tex_cnt, const a_tex_index_to_file_path_mapper tex_index_to_file_path, const size_t font_cnt, const a_font_index_to_info_mapper font_index_to_info, s_mem_arena& mem_arena, s_mem_arena& scratch_space) {
        assert(IsDefault(*this));

        if (tex_cnt > 0) {
            textures = PushTextures(tex_cnt, tex_index_to_file_path, mem_arena);

            if (textures.cnt == 0) {
                return false;
            }
        }

        if (font_cnt > 0) {
            fonts = PushFonts(font_cnt, font_index_to_info, mem_arena, scratch_space);

            if (fonts.cnt == 0) {
                return false;
            }
        }

        textured_quad_shader_prog = LoadTexturedQuadShaderProg();
        batch_gl_ids = GenBatchGLBufs();
        surf_gl_ids = GenSurfGLBufs();

        return true;
    }

    void s_pers_render_data::Clean() {
        assert(IsValid());

        GL_CALL(glDeleteBuffers(1, &batch_gl_ids.elem_buf_gl_id));
        GL_CALL(glDeleteBuffers(1, &batch_gl_ids.vert_buf_gl_id));
        GL_CALL(glDeleteVertexArrays(1, &batch_gl_ids.vert_array_gl_id));

        GL_CALL(glDeleteBuffers(1, &surf_gl_ids.elem_buf_gl_id));
        GL_CALL(glDeleteBuffers(1, &surf_gl_ids.vert_buf_gl_id));
        GL_CALL(glDeleteVertexArrays(1, &surf_gl_ids.vert_array_gl_id));

        glDeleteTextures(fonts.tex_gl_ids.len, fonts.tex_gl_ids.elems_raw);
        glDeleteTextures(textures.gl_ids.len, textures.gl_ids.elems_raw);

        *this = {};
    }

    bool s_surfaces::Init(const int cnt, const s_vec_2d_i size) {
        assert(cnt > 0 && cnt <= g_surface_limit);
        assert(size.x > 0 && size.y > 0);

        GL_CALL(glGenFramebuffers(cnt, framebuffer_gl_ids.elems_raw));
        GL_CALL(glGenTextures(cnt, framebuffer_tex_gl_ids.elems_raw));

        this->cnt = cnt;

        for (int i = 0; i < cnt; ++i) {
            if (!AttachFramebufferTexture(framebuffer_gl_ids[i], framebuffer_tex_gl_ids[i], size)) {
                Clean();
                return false;
            }
        }

        return true;
    }

    void s_surfaces::Clean() {
        GL_CALL(glDeleteTextures(cnt, framebuffer_tex_gl_ids.elems_raw));
        GL_CALL(glDeleteFramebuffers(cnt, framebuffer_gl_ids.elems_raw));

        *this = {};
    }

    bool s_surfaces::Resize(const s_vec_2d_i size) {
        assert(size.x > 0 && size.y > 0);

        if (cnt > 0) {
            glDeleteTextures(cnt, framebuffer_tex_gl_ids.elems_raw);
            glGenTextures(cnt, framebuffer_tex_gl_ids.elems_raw);

            for (int i = 0; i < cnt; ++i) {
                if (!AttachFramebufferTexture(framebuffer_gl_ids[i], framebuffer_tex_gl_ids[i], size)) {
                    return false;
                }
            }
        }

        return true;
    }

    s_str_draw_info GenStrDrawInfo(const char* const str, const int font_index, const s_fonts& fonts, const s_vec_2d pos, const e_str_hor_align hor_align, const e_str_ver_align ver_align) {
        assert(str && str[0]);

        s_str_draw_info draw_info;
        draw_info.line_infos.len = 1;

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

                draw_info.chr_draw_rects.Last() = {chr_draw_pos, font_arrangement_info.chrs.src_rects[chr_index].Size()};

                chr_draw_pos_pen.x += font_arrangement_info.chrs.hor_advances[chr_index];
            } else {
                draw_info.line_infos.Last().width_including_offs = chr_draw_pos_pen.x;
                ++draw_info.line_infos.len;
                draw_info.line_infos.Last().begin_chr_index = draw_info.chr_draw_rects.len;

                chr_draw_pos_pen.x = 0;
                chr_draw_pos_pen.y += font_arrangement_info.line_height;
            }
        } while (str[draw_info.chr_draw_rects.len]);

        draw_info.line_infos.Last().width_including_offs = chr_draw_pos_pen.x;
        const float height_including_offs = chr_draw_pos_pen.y + font_arrangement_info.line_height;

        //
        // Second Pass: Applying Position and Alignment Offsets
        //
        for (int i = 0; i < draw_info.line_infos.len; i++) {
            const int line_end_chr_index = i < draw_info.line_infos.len - 1 ? draw_info.line_infos[i + 1].begin_chr_index - 1 : draw_info.chr_draw_rects.len - 1;

            const float hor_align_offs = -(draw_info.line_infos[i].width_including_offs * static_cast<float>(hor_align) * 0.5f);

            const s_vec_2d offs = pos + s_vec_2d(hor_align_offs, -(height_including_offs * static_cast<float>(ver_align) * 0.5f));

            for (int j = draw_info.line_infos[i].begin_chr_index; j <= line_end_chr_index; ++j) {
                draw_info.chr_draw_rects[j] = draw_info.chr_draw_rects[j].Translated(offs);
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
                bottom = Max(bottom, draw_info.chr_draw_rects[i].Bottom());
            }
        } else {
            for (int i = 0; i < draw_info.line_infos[1].begin_chr_index; ++i) {
                top = Min(top, draw_info.chr_draw_rects[i].y);
            }

            for (int i = draw_info.line_infos.Last().begin_chr_index; i < str_len; ++i) {
                bottom = Max(bottom, draw_info.chr_draw_rects[i].Bottom());
            }
        }

        // Get left and right.
        float left = std::numeric_limits<float>().max();
        float right = std::numeric_limits<float>().lowest();

        for (int i = 0; i < line_cnt; ++i) {
            const int begin_chr_index = draw_info.line_infos[i].begin_chr_index;
            left = Min(left, draw_info.chr_draw_rects[begin_chr_index].x);

            const int end_chr_index = i < line_cnt - 1 ? draw_info.line_infos[i + 1].begin_chr_index : str_len - 1;
            right = Max(right, draw_info.chr_draw_rects[end_chr_index].Right());
        }

        return {left, top, right - left, bottom - top};
    }

    bool ExecDrawInstrs(const s_array<const a_draw_instr> instrs, const s_vec_2d_i window_size, const s_pers_render_data& pers_render_data, const s_surfaces& surfs, s_mem_arena& scratch_space) {
        assert(instrs.IsValid());
        assert(window_size.x > 0 && window_size.y > 0);
        assert(surfs.IsValid());
        assert(scratch_space.IsInitialized());

        const auto batch_verts = PushArray<float>(g_batch_slot_vert_cnt * g_batch_slot_limit, scratch_space);

        if (batch_verts.IsInitialized()) {
            return false;
        }

        size_t batch_slots_used_cnt = 0;
        a_gl_id batch_tex_gl_id = 0; // NOTE: In the future we will support multiple texture units. This is just for simplicity right now.

        a_gl_id surf_shader_prog_gl_id = 0; // When we draw a surface, it will use this shader program.
        s_static_list<int, g_surface_limit> surf_index_stack;

        const auto proj_mat = s_matrix_4x4::GenOrtho(0.0f, window_size.x, window_size.y, 0.0f, -1.0f, 1.0f);
        s_matrix_4x4 view_mat;

        // NOTE: The blend setup could be moved into game initialisation.
        GL_CALL(glEnable(GL_BLEND));
        GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        for (int i = 0; i < instrs.len; ++i) {
            //
            // Instruction Processing
            //
            std::visit([&](const auto& instr) {
                using T = std::decay_t<decltype(instr)>;

                if constexpr (std::is_same_v<T, s_clear_instr>) {
                    glClearColor(instr.color.x, instr.color.y, instr.color.z, instr.color.w);
                    glClear(GL_COLOR_BUFFER_BIT);
                } else if constexpr (std::is_same_v<T, s_set_view_matrix_instr>) {
                    view_mat = instr.mat;
                } else if constexpr (std::is_same_v<T, s_draw_texture_instr>) {
                    static_assert(g_batch_slot_limit > 0);

                    if (batch_slots_used_cnt == 0) {
                        batch_tex_gl_id = instr.tex_gl_id;
                    }

                    const s_array<float> slot_verts = {batch_verts.elems_raw + (g_batch_slot_vert_cnt * batch_slots_used_cnt), g_batch_slot_vert_cnt};
                    WriteVerts(slot_verts, instr.tex_gl_id, instr.tex_coords, instr.pos, instr.size, instr.origin, instr.rot, instr.blend);

                    ++batch_slots_used_cnt;
                } else if constexpr (std::is_same_v<T, s_set_surf_instr>) {
                    assert(instr.surf_index >= 0 && instr.surf_index < surfs.cnt);

                    // Add the surface index to the stack.
                    if (!surf_index_stack.Append(instr.surf_index)) {
                        //return false;
                    }

                    // Bind the surface framebuffer.
                    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, surfs.framebuffer_gl_ids[instr.surf_index]));
                } else if constexpr (std::is_same_v<T, s_unset_surf_instr>) {
                    surf_index_stack.Pop();

                    if (surf_index_stack.len == 0) {
                        GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
                    } else {
                        const int new_surf_index = surf_index_stack.Last();
                        const GLuint fb_gl_id = surfs.framebuffer_gl_ids[new_surf_index];
                        GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, surfs.framebuffer_gl_ids[new_surf_index]));
                    }
                } else if constexpr (std::is_same_v<T, s_draw_surf_instr>) {
                    assert(instr.surf_index >= 0 && instr.surf_index < surfs.cnt);
                    assert(surf_shader_prog_gl_id); // Make sure the surface shader program has been set.

                    GL_CALL(glUseProgram(surf_shader_prog_gl_id));
                    GL_CALL(glActiveTexture(GL_TEXTURE0));
                    GL_CALL(glBindTexture(GL_TEXTURE_2D, surfs.framebuffer_tex_gl_ids[instr.surf_index]));
                    GL_CALL(glBindVertexArray(pers_render_data.surf_gl_ids.vert_array_gl_id));
                    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pers_render_data.surf_gl_ids.elem_buf_gl_id));
                    GL_CALL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr));
                } else if constexpr (std::is_same_v<T, s_set_surf_shader_prog_uniform_instr>) {
                    assert(surf_shader_prog_gl_id); // Make sure the surface shader program has been set.

                    glUseProgram(surf_shader_prog_gl_id);

                    const int uni_loc = glGetUniformLocation(surf_shader_prog_gl_id, instr.name);
                    assert(uni_loc != -1);

                    // std::variant<int, float, s_vec_2d, s_vec_3d, s_vec_4d, s_matrix_4x4> val;

#if 0
                    glUseProgram(surf_shader_prog_gl_id);

                    const int uni_loc = glGetUniformLocation(surf_shader_prog_gl_id, instr.SetSurfShaderProgUniform().name);
                    assert(uni_loc != -1);

                    switch (instr.SetSurfShaderProgUniform().val_type) {
                        case ek_shader_uniform_val_type_int:
                            GL_CALL(glUniform1i(uni_loc, instr.SetSurfShaderProgUniform().ValAsInt()));
                            break;

                        case ek_shader_uniform_val_type_float:
                            GL_CALL(glUniform1f(uni_loc, instr.SetSurfShaderProgUniform().ValAsFloat()));
                            break;

                        case ek_shader_uniform_val_type_v2:
                            GL_CALL(glUniform2fv(uni_loc, 1, reinterpret_cast<const float*>(&instr.SetSurfShaderProgUniform().ValAsV2()));
                            break;

                        case ek_shader_uniform_val_type_v3:
                            GL_CALL(glUniform3fv(uni_loc, 1, reinterpret_cast<const float*>(&instr.SetSurfShaderProgUniform().ValAsV3()));
                            break;

                        case ek_shader_uniform_val_type_v4:
                            GL_CALL(glUniform4fv(uni_loc, 1, reinterpret_cast<const float*>(&instr.SetSurfShaderProgUniform().ValAsV4()));
                            break;

                        case ek_shader_uniform_val_type_mat_4x4:
                            GL_CALL(glUniformMatrix4fv(uni_loc, 1, false, reinterpret_cast<const float*>(&instr.SetSurfShaderProgUniform().ValAsMat4x4()));
                            break;
                    }
#endif
                }
            }, instrs[i]);

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
                const a_draw_instr& instr_next = instrs[i + 1];

                if (batch_slots_used_cnt == g_batch_slot_limit
                    || std::holds_alternative<s_set_view_matrix_instr>(instr_next)
                    || std::holds_alternative<s_set_surf_instr>(instr_next)
                    || std::holds_alternative<s_unset_surf_instr>(instr_next)
                    || (std::holds_alternative<s_draw_texture_instr>(instr_next) && std::get_if<s_draw_texture_instr>(&instr_next)->tex_gl_id != batch_tex_gl_id)) {
                    flush = true;
                }
            }

            if (flush) {
                // Write the batch vertex data to the GPU.
                GL_CALL(glBindVertexArray(pers_render_data.batch_gl_ids.vert_array_gl_id));
                GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, pers_render_data.batch_gl_ids.vert_buf_gl_id));

                const size_t write_size = g_batch_slot_verts_size * batch_slots_used_cnt;
                GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, write_size, batch_verts.elems_raw));

                // Draw the batch.
                const s_textured_quad_shader_prog& prog = pers_render_data.textured_quad_shader_prog;

                GL_CALL(glUseProgram(prog.gl_id));

                GL_CALL(glUniformMatrix4fv(prog.proj_uniform_loc, 1, false, reinterpret_cast<const float*>(&proj_mat)));
                GL_CALL(glUniformMatrix4fv(prog.view_uniform_loc, 1, false, reinterpret_cast<const float*>(&view_mat)));

                GL_CALL(glBindTexture(GL_TEXTURE_2D, batch_tex_gl_id));

                GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pers_render_data.batch_gl_ids.elem_buf_gl_id));
                GL_CALL(glDrawElements(GL_TRIANGLES, g_batch_slot_indices_cnt * batch_slots_used_cnt, GL_UNSIGNED_SHORT, nullptr));

                // Clear batch state.
                batch_slots_used_cnt = 0;
                batch_tex_gl_id = 0;
            }
        }

        return true;
    }

    bool AppendDrawTextureInstr(s_list<a_draw_instr>& instrs, const int tex_index, const s_textures& textures, const s_rect_i src_rect, const s_vec_2d pos, const s_vec_2d origin, const s_vec_2d scale, const float rot, const s_vec_4d blend) {
        assert(instrs.IsInitialized());
        assert(textures.IsValid());
        assert(tex_index >= 0 && tex_index < textures.cnt);
        assert(IsColorValid(blend));

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

        return instrs.Append(draw_tex);
    }

    bool AppendDrawTextureInstrsForStr(s_list<a_draw_instr>& instrs, const char* const str, const int font_index, const s_fonts& fonts, const s_vec_2d pos, const s_vec_4d blend, const e_str_hor_align hor_align, const e_str_ver_align ver_align) {
        assert(instrs.IsInitialized());
        assert(str);
        assert(fonts.IsValid());
        assert(font_index >= 0 && font_index < fonts.cnt);
        assert(IsColorValid(blend));

        const s_str_draw_info str_draw_info = GenStrDrawInfo(str, font_index, fonts, pos, hor_align, ver_align); // TODO: Cache this, but only if it's actually a bottleneck on performance.

        const s_font_arrangement_info& font_arrangement_info = fonts.arrangement_infos[font_index];
        const GLuint font_tex_gl_id = fonts.tex_gl_ids[font_index];
        const s_vec_2d_i font_tex_size = {g_font_texture_width, fonts.tex_heights[font_index]};

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
                .pos = str_draw_info.chr_draw_rects[i].TopLeft(),
                .size = str_draw_info.chr_draw_rects[i].Size(),
                .blend = blend
            };

            if (!instrs.Append(draw_tex)) {
                return false;
            }
        }

        return true;
    }
}
