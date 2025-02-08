#include <zf4_assets.h>

#include <cstdio>
#include <cstdlib>
#include <AL/alext.h>
#include <zf4_utils.h>

namespace zf4 {
    static inline int TexturePixelDataSize(const s_vec_2d_i tex_size) {
        return g_texture_channel_cnt * tex_size.x * tex_size.y;
    }

    static void SetUpGLTexture(const GLuint gl_id, const s_vec_2d_i size, const s_array<const unsigned char> px_data) {
        assert(gl_id);
        assert(TexturePixelDataSize(size) <= px_data.len);

        ZF4_GL_CALL(glBindTexture(GL_TEXTURE_2D, gl_id));
        ZF4_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        ZF4_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        ZF4_GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, px_data.elems_raw));
    }

    static bool LoadTexturesFromFS(s_textures& textures, FILE* const fs, s_mem_arena& scratch_space) {
        assert(IsStructZero(textures));

        // Save the offset in the scratch space arena where we started at, so we can revert back at the end of the function.
        const int scratch_space_begin_offs = scratch_space.offs;

        // Read and verify texture count.
        fread(&textures.cnt, sizeof(textures.cnt), 1, fs);
        assert(textures.cnt >= 0 && textures.cnt <= g_texture_limit);

        if (textures.cnt > 0) {
            // Reserve scratch space memory for pixel data, to be reused for all textures.
            const auto px_data = PushArray<unsigned char>(g_texture_px_data_size_limit, scratch_space);

            if (IsStructZero(px_data)) {
                return false;
            }

            // Load textures.
            ZF4_GL_CALL(glGenTextures(textures.cnt, textures.gl_ids.elems_raw));

            for (int i = 0; i < textures.cnt; ++i) {
                fread(&textures.sizes[i], sizeof(textures.sizes[i]), 1, fs);

                const int tex_px_data_size = TexturePixelDataSize(textures.sizes[i]);
                fread(px_data.elems_raw, tex_px_data_size, 1, fs);

                SetUpGLTexture(textures.gl_ids[i], textures.sizes[i], px_data);
            }
        }

        // Clear out the memory in the scratch space that we used in this function.
        RewindMemArena(scratch_space, scratch_space_begin_offs);

        return true;
    }

    static bool LoadFontsFromFS(s_fonts& fonts, FILE* const fs, s_mem_arena& scratch_space) {
        assert(IsStructZero(fonts));

        // Save the offset in the scratch space arena where we started at, so we can revert back at the end of the function.
        const int scratch_space_begin_offs = scratch_space.offs;

        // Read and verify font count.
        fread(&fonts.cnt, sizeof(fonts.cnt), 1, fs);
        assert(fonts.cnt >= 0 && fonts.cnt <= g_font_limit);

        if (fonts.cnt > 0) {
            // Reserve scratch space memory for pixel data, to be reused for all font textures.
            const auto px_data = PushArray<unsigned char>(g_texture_px_data_size_limit, scratch_space);

            if (IsStructZero(px_data)) {
                return false;
            }

            // Load fonts.
            ZF4_GL_CALL(glGenTextures(fonts.cnt, fonts.tex_gl_ids.elems_raw));

            for (int i = 0; i < fonts.cnt; ++i) {
                fread(&fonts.arrangement_infos[i], sizeof(fonts.arrangement_infos[i]), 1, fs);
                fread(&fonts.tex_sizes[i], sizeof(fonts.tex_sizes[i]), 1, fs);
                fread(px_data.elems_raw, g_texture_px_data_size_limit, 1, fs);
                SetUpGLTexture(fonts.tex_gl_ids[i], fonts.tex_sizes[i], px_data);
            }
        }

        // Clear out the memory in the scratch space that we used in this function.
        RewindMemArena(scratch_space, scratch_space_begin_offs);

        return true;
    }

    static bool LoadShaderProgsFromFS(s_shader_progs& progs, FILE* const fs, s_mem_arena& scratch_space) {
        assert(IsStructZero(progs));

        // Save the offset in the scratch space arena where we started at, so we can revert back at the end of the function.
        const int scratch_space_begin_offs = scratch_space.offs;

        // Read and verify the shader program count.
        fread(&progs.cnt, sizeof(progs.cnt), 1, fs);
        assert(progs.cnt >= 0 && progs.cnt <= g_shader_prog_limit);

        if (progs.cnt > 0) {
            // Reserve memory to store shader source code.
            const auto vert_shader_src_buf = PushArray<char>(g_shader_src_len_limit + 1, scratch_space);

            if (IsStructZero(vert_shader_src_buf)) {
                return false;
            }

            const auto frag_shader_src_buf = PushArray<char>(g_shader_src_len_limit + 1, scratch_space);

            if (IsStructZero(frag_shader_src_buf)) {
                return false;
            }

            for (int i = 0; i < progs.cnt; ++i) {
                // Get vertex shader source.
                int vert_shader_src_len;
                fread(&vert_shader_src_len, sizeof(vert_shader_src_len), 1, fs);
                fread(vert_shader_src_buf.elems_raw, vert_shader_src_len, 1, fs);
                vert_shader_src_buf[vert_shader_src_len] = '\0';

                // Get fragment shader source.
                int frag_shader_src_len;
                fread(&frag_shader_src_len, sizeof(frag_shader_src_len), 1, fs);
                fread(frag_shader_src_buf.elems_raw, frag_shader_src_len, 1, fs);
                frag_shader_src_buf[frag_shader_src_len] = '\0';

                // Create the program.
                progs.gl_ids[i] = CreateShaderProgFromSrcs(vert_shader_src_buf.elems_raw, frag_shader_src_buf.elems_raw);

                if (!progs.gl_ids[i]) {
                    return false;
                }
            }
        }

        return true;
    }

    static bool LoadSoundsFromFS(s_sounds& snds, FILE* const fs, s_mem_arena& scratch_space) {
        assert(IsStructZero(snds));

        // Save the offset in the scratch space arena where we started at, so we can revert back at the end of the function.
        const int scratch_space_begin_offs = scratch_space.offs;

        // Read and verify sound count.
        fread(&snds.cnt, sizeof(snds.cnt), 1, fs);
        assert(snds.cnt >= 0 && snds.cnt <= g_sound_limit);

        if (snds.cnt > 0) {
            alGenBuffers(snds.cnt, snds.buf_al_ids.elems_raw);

            const auto samples = PushArray<ta_audio_sample>(g_sound_sample_limit, scratch_space);

            for (int i = 0; i < snds.cnt; ++i) {
                s_audio_info audio_info;
                fread(&audio_info, sizeof(audio_info), 1, fs);

                const long long sample_cnt = audio_info.sample_cnt_per_channel * audio_info.channel_cnt;
                fread(samples.elems_raw, sizeof(ta_audio_sample), sample_cnt, fs);

                const ALenum format = audio_info.channel_cnt == 1 ? AL_FORMAT_MONO_FLOAT32 : AL_FORMAT_STEREO_FLOAT32;
                alBufferData(snds.buf_al_ids[i], format, samples.elems_raw, sizeof(ta_audio_sample) * sample_cnt, audio_info.sample_rate);
            }
        }

        return true;
    }

    static bool LoadMusicFromFS(s_music& music, FILE* const fs) {
        assert(IsStructZero(music));

        fread(&music.cnt, sizeof(music.cnt), 1, fs);
        assert(music.cnt >= 0 && music.cnt <= g_music_limit);

        for (int i = 0; i < music.cnt; ++i) {
            fread(&music.infos[i], sizeof(s_audio_info), 1, fs);
            music.sample_data_file_positions[i] = ftell(fs);
        }

        return true;
    }

    s_assets* LoadAssets(s_mem_arena& mem_arena, s_mem_arena& scratch_space) {
        // Push the asset data to the memory arena.
        const auto assets = PushType<s_assets>(mem_arena);

        if (!assets) {
            LogError("Failed to reserve memory for assets!");
            return nullptr;
        }

        // Open the assets file.
        FILE* const fs = fopen(g_assets_file_name, "rb");

        if (!fs) {
            LogError("Failed to open \"%s\"!", g_assets_file_name);
            return nullptr;
        }

        // Load the assets of each type using the file.
        bool success = false;

        do {
            if (!LoadTexturesFromFS(assets->textures, fs, scratch_space)) {
                LogError("Failed to load textures!");
                break;
            }

            if (!LoadFontsFromFS(assets->fonts, fs, scratch_space)) {
                LogError("Failed to load fonts!");
                break;
            }

            if (!LoadShaderProgsFromFS(assets->shader_progs, fs, scratch_space)) {
                LogError("Failed to load shader programs!");
                break;
            }

            if (!LoadSoundsFromFS(assets->sounds, fs, scratch_space)) {
                LogError("Failed to load sounds!");
                break;
            }

            if (!LoadMusicFromFS(assets->music, fs)) {
                LogError("Failed to load music!");
                break;
            }

            success = true;
        } while (false);

        fclose(fs);

        return success ? assets : nullptr;
    }

    void UnloadAssets(s_assets& assets) {
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
