#include <zf4_assets.h>

#include <cstdio>
#include <cstdlib>
#include <AL/alext.h>
#include <zf4_utils.h>

namespace zf4 {
    static inline int TexturePixelDataSize(const s_vec_2d_i tex_size) {
        return g_texture_channel_cnt * tex_size.x * tex_size.y;
    }

    static void SetUpGeneratedGLTexture(const GLuint gl_id, const s_vec_2d_i size, const s_array<const unsigned char> px_data) {
        assert(gl_id);
        assert(TexturePixelDataSize(size) <= px_data.len);

        ZF4_GL_CALL(glBindTexture(GL_TEXTURE_2D, gl_id));
        ZF4_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        ZF4_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        ZF4_GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, px_data.elems_raw));
    }

    static bool LoadUserTexturesFromFS(s_textures& textures, FILE* const fs, s_mem_arena& mem_arena, s_mem_arena& scratch_space) {
        assert(IsStructZero(textures));

        const int scratch_space_begin_offs = scratch_space.offs;

        const auto tex_cnt = ReadFromFS<int>(fs);

        if (tex_cnt > 0) {
            const auto gl_ids = PushArray<GLuint>(tex_cnt, mem_arena);
            const auto sizes = PushArray<s_vec_2d_i>(tex_cnt, mem_arena);

            if (IsStructZero(gl_ids) || IsStructZero(sizes)) {
                return false;
            }

            const auto px_data = PushArray<unsigned char>(g_texture_px_data_size_limit, scratch_space);

            if (IsStructZero(px_data)) {
                return false;
            }

            ZF4_GL_CALL(glGenTextures(tex_cnt, gl_ids.elems_raw));

            for (int i = 0; i < tex_cnt; ++i) {
                fread(&sizes[i], sizeof(sizes[i]), 1, fs);

                const int tex_px_data_size = TexturePixelDataSize(sizes[i]);
                fread(px_data.elems_raw, tex_px_data_size, 1, fs);

                SetUpGeneratedGLTexture(gl_ids[i], sizes[i], px_data);
            }

            textures.gl_ids = gl_ids;
            textures.sizes = sizes;
            textures.cnt = tex_cnt;
        }

        RewindMemArena(scratch_space, scratch_space_begin_offs);

        return true;
    }

    static bool LoadUserFontsFromFS(s_fonts& fonts, FILE* const fs, s_mem_arena& mem_arena, s_mem_arena& scratch_space) {
        assert(IsStructZero(fonts));

        const int scratch_space_begin_offs = scratch_space.offs;

        const auto font_cnt = ReadFromFS<int>(fs);

        if (font_cnt > 0) {
            const auto arrangement_infos = PushArray<s_font_arrangement_info>(font_cnt, mem_arena);
            const auto tex_gl_ids = PushArray<GLuint>(font_cnt, mem_arena);
            const auto tex_sizes = PushArray<s_vec_2d_i>(font_cnt, mem_arena);

            if (IsStructZero(arrangement_infos) || IsStructZero(tex_gl_ids) || IsStructZero(tex_sizes)) {
                return false;
            }

            const auto px_data = PushArray<unsigned char>(g_texture_px_data_size_limit, scratch_space);

            if (IsStructZero(px_data)) {
                return false;
            }

            ZF4_GL_CALL(glGenTextures(font_cnt, tex_gl_ids.elems_raw));

            for (int i = 0; i < font_cnt; ++i) {
                fread(&arrangement_infos[i], sizeof(arrangement_infos[i]), 1, fs);
                fread(&tex_sizes[i], sizeof(tex_sizes[i]), 1, fs);
                fread(px_data.elems_raw, g_texture_px_data_size_limit, 1, fs);
                SetUpGeneratedGLTexture(tex_gl_ids[i], tex_sizes[i], px_data);
            }

            fonts.arrangement_infos = arrangement_infos;
            fonts.tex_gl_ids = tex_gl_ids;
            fonts.tex_sizes = tex_sizes;
            fonts.cnt = font_cnt;
        }

        RewindMemArena(scratch_space, scratch_space_begin_offs);

        return true;
    }

    static bool LoadUserShaderProgsFromFS(s_shader_progs& progs, FILE* const fs, s_mem_arena& mem_arena, s_mem_arena& scratch_space) {
        assert(IsStructZero(progs));

        const int scratch_space_begin_offs = scratch_space.offs;

        progs.cnt = ReadFromFS<int>(fs);

        if (progs.cnt > 0) {
            const auto gl_ids = PushArray<GLuint>(progs.cnt, mem_arena);

            if (IsStructZero(gl_ids)) {
                return false;
            }

            const auto vert_shader_src_buf = PushArray<char>(g_shader_src_len_limit + 1, scratch_space);
            const auto frag_shader_src_buf = PushArray<char>(g_shader_src_len_limit + 1, scratch_space);

            if (IsStructZero(vert_shader_src_buf) || IsStructZero(frag_shader_src_buf)) {
                return false;
            }

            for (int i = 0; i < progs.cnt; ++i) {
                const auto vert_shader_src_len = ReadFromFS<int>(fs);
                fread(vert_shader_src_buf.elems_raw, vert_shader_src_len, 1, fs);
                vert_shader_src_buf[vert_shader_src_len] = '\0';

                const auto frag_shader_src_len = ReadFromFS<int>(fs);
                fread(frag_shader_src_buf.elems_raw, frag_shader_src_len, 1, fs);
                frag_shader_src_buf[frag_shader_src_len] = '\0';

                gl_ids[i] = CreateShaderProgFromSrcs(vert_shader_src_buf.elems_raw, frag_shader_src_buf.elems_raw);

                if (!gl_ids[i]) {
                    for (int j = 0; j < i; ++j) {
                        glDeleteProgram(gl_ids[j]);
                    }

                    return false;
                }
            }

            progs.gl_ids = gl_ids;
        }

        return true;
    }

    static bool LoadUserSoundsFromFS(s_sounds& sounds, FILE* const fs, s_mem_arena& mem_arena, s_mem_arena& scratch_space) {
        assert(IsStructZero(sounds));

        const int scratch_space_begin_offs = scratch_space.offs;

        sounds.cnt = ReadFromFS<int>(fs);

        if (sounds.cnt > 0) {
            const auto buf_al_ids = PushArray<ALuint>(sounds.cnt, mem_arena);

            if (IsStructZero(buf_al_ids)) {
                return false;
            }

            const auto samples = PushArray<ta_audio_sample>(g_sound_sample_limit, scratch_space);

            if (IsStructZero(samples)) {
                return false;
            }

            alGenBuffers(sounds.cnt, buf_al_ids.elems_raw);

            for (int i = 0; i < sounds.cnt; ++i) {
                const auto audio_info = ReadFromFS<s_audio_info>(fs);

                const long long sample_cnt = audio_info.sample_cnt_per_channel * audio_info.channel_cnt;
                fread(samples.elems_raw, sizeof(ta_audio_sample), sample_cnt, fs);

                const ALenum format = audio_info.channel_cnt == 1 ? AL_FORMAT_MONO_FLOAT32 : AL_FORMAT_STEREO_FLOAT32;
                alBufferData(buf_al_ids[i], format, samples.elems_raw, sizeof(ta_audio_sample) * sample_cnt, audio_info.sample_rate);
            }

            sounds.buf_al_ids = buf_al_ids;
        }

        RewindMemArena(scratch_space, scratch_space_begin_offs);

        return true;
    }

    static bool LoadUserMusicFromFS(s_music& music, FILE* const fs, s_mem_arena& mem_arena) {
        const auto music_cnt = ReadFromFS<int>(fs);

        if (music_cnt > 0) {
            const auto infos = PushArray<s_audio_info>(music_cnt, mem_arena);
            const auto sample_data_file_positions = PushArray<int>(music_cnt, mem_arena);

            if (IsStructZero(infos) || IsStructZero(sample_data_file_positions)) {
                return false;
            }

            for (int i = 0; i < music_cnt; ++i) {
                infos[i] = ReadFromFS<s_audio_info>(fs);
                sample_data_file_positions[i] = ftell(fs);
            }

            music.infos = infos;
            music.sample_data_file_positions = sample_data_file_positions;

            music.cnt = music_cnt;
        }

        return true;
    }

    bool LoadUserAssets(s_user_assets& assets, s_mem_arena& mem_arena, s_mem_arena& scratch_space) {
        assert(IsStructZero(assets));

        // Open the assets file.
        FILE* const fs = fopen(g_user_assets_file_name, "rb");

        if (!fs) {
            LogError("Failed to open \"%s\"!", g_user_assets_file_name);
            return false;
        }

        // Load the assets of each type using the file.
        bool success = false;

        do {
            if (!LoadUserTexturesFromFS(assets.textures, fs, mem_arena, scratch_space)) {
                LogError("Failed to load textures!");
                break;
            }

            if (!LoadUserFontsFromFS(assets.fonts, fs, mem_arena, scratch_space)) {
                LogError("Failed to load fonts!");
                break;
            }

            if (!LoadUserShaderProgsFromFS(assets.shader_progs, fs, mem_arena, scratch_space)) {
                LogError("Failed to load shader programs!");
                break;
            }

            if (!LoadUserSoundsFromFS(assets.sounds, fs, mem_arena, scratch_space)) {
                LogError("Failed to load sounds!");
                break;
            }

            if (!LoadUserMusicFromFS(assets.music, fs, mem_arena)) {
                LogError("Failed to load music!");
                break;
            }

            success = true;
        } while (false);

        fclose(fs);

        return success;
    }

    void UnloadUserAssets(s_user_assets& assets) {
        if (assets.sounds.cnt > 0) {
            alDeleteBuffers(assets.sounds.cnt, assets.sounds.buf_al_ids.elems_raw);
        }

        for (int i = 0; i < assets.shader_progs.cnt; ++i) {
            ZF4_GL_CALL(glDeleteProgram(assets.shader_progs.gl_ids[i]));
        }

        if (assets.fonts.cnt > 0) {
            ZF4_GL_CALL(glDeleteTextures(assets.fonts.cnt, assets.fonts.tex_gl_ids.elems_raw));
        }

        if (assets.textures.cnt > 0) {
            ZF4_GL_CALL(glDeleteTextures(assets.textures.cnt, assets.textures.gl_ids.elems_raw));
        }

        ZeroOutStruct(assets);
    }

    s_builtin_assets LoadBuiltinAssets() {
        s_builtin_assets assets = {};

        // Generate pixel texture.
        glGenTextures(1, &assets.pixel_tex_gl_id);

        s_static_array<unsigned char, 4> pixel_tex_px_data = {};
        pixel_tex_px_data[0] = 255;
        pixel_tex_px_data[1] = 255;
        pixel_tex_px_data[2] = 255;
        pixel_tex_px_data[3] = 255;

        SetUpGeneratedGLTexture(assets.pixel_tex_gl_id, {1, 1}, pixel_tex_px_data);

        // Generate blend shader program.
        {
            const char* const blend_vert_shader = R"(#version 430 core

layout (location = 0) in vec2 a_vert;
layout (location = 1) in vec2 a_tex_coord;

out vec2 v_tex_coord;

void main() {
    gl_Position = vec4(a_vert, 0.0f, 1.0f);
    v_tex_coord = a_tex_coord;
}
)";

            const char* const blend_frag_shader = R"(#version 430 core

in vec2 v_tex_coord;
out vec4 o_frag_color;

uniform sampler2D u_tex;

uniform vec3 u_color;
uniform float u_intensity;

void main() {
    vec4 color = texture(u_tex, v_tex_coord);

    o_frag_color = vec4(
        mix(color.r, u_color.r, u_intensity),
        mix(color.g, u_color.g, u_intensity),
        mix(color.b, u_color.b, u_intensity),
        color.a
    );
}
)";

            assets.blend_shader_prog_gl_id = CreateShaderProgFromSrcs(blend_vert_shader, blend_frag_shader);
        }

        return assets;
    }

    void UnloadBuiltinAssets(s_builtin_assets& assets) {
        glDeleteTextures(1, &assets.pixel_tex_gl_id);
        glDeleteProgram(assets.blend_shader_prog_gl_id);

        ZeroOutStruct(assets);
    }

    GLuint CreateShaderFromSrc(const char* const src, const bool frag) {
        const GLuint gl_id = ZF4_GL_CALL(glCreateShader(frag ? GL_FRAGMENT_SHADER : GL_VERTEX_SHADER));
        ZF4_GL_CALL(glShaderSource(gl_id, 1, &src, nullptr));
        ZF4_GL_CALL(glCompileShader(gl_id));

        GLint compile_success;
        ZF4_GL_CALL(glGetShaderiv(gl_id, GL_COMPILE_STATUS, &compile_success));

        if (!compile_success) {
            ZF4_GL_CALL(glDeleteShader(gl_id));
            return 0;
        }

        return gl_id;
    }

    GLuint CreateShaderProgFromSrcs(const char* const vert_shader_src, const char* const frag_shader_src) {
        const GLuint vert_shader_gl_id = CreateShaderFromSrc(vert_shader_src, false);

        if (!vert_shader_gl_id) {
            return 0;
        }

        const GLuint frag_shader_gl_id = CreateShaderFromSrc(frag_shader_src, true);

        if (!frag_shader_gl_id) {
            ZF4_GL_CALL(glDeleteShader(vert_shader_gl_id));
            return 0;
        }

        const GLuint prog_gl_id = ZF4_GL_CALL(glCreateProgram());
        ZF4_GL_CALL(glAttachShader(prog_gl_id, vert_shader_gl_id));
        ZF4_GL_CALL(glAttachShader(prog_gl_id, frag_shader_gl_id));
        ZF4_GL_CALL(glLinkProgram(prog_gl_id));

        // We no longer need the shaders, as they are now part of the program.
        ZF4_GL_CALL(glDeleteShader(vert_shader_gl_id));
        ZF4_GL_CALL(glDeleteShader(frag_shader_gl_id));

        return prog_gl_id;
    }
}
