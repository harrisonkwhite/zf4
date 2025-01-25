#include <zf4_assets.h>

#include <stdlib.h>
#include <AL/alext.h>
#include <zf4_utils.h>

static inline int TexturePixelDataSize(const s_vec_2d_i tex_size) {
    assert(tex_size.x > 0 && tex_size.y > 0);
    return TEXTURE_CHANNEL_CNT * tex_size.x * tex_size.y;
}

static void SetUpGLTexture(const GLuint gl_id, const s_vec_2d_i size, const unsigned char* const px_data) {
    assert(gl_id);
    assert(size.x > 0 && size.y > 0);
    assert(px_data);

    GL_CALL(glBindTexture(GL_TEXTURE_2D, gl_id));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, px_data));
}

static bool LoadTexturesFromFS(s_textures* const textures, FILE* const fs, s_mem_arena* const scratch_space) {
    assert(textures);
    assert(IsClear(textures, sizeof(*textures)));
    assert(fs);
    assert(IsMemArenaValid(scratch_space));

    // Save the offset in the scratch space arena where we started at, so we can revert back at the end of the function.
    const int scratch_space_begin_offs = scratch_space->offs;

    // Read and verify texture count.
    fread(&textures->cnt, sizeof(textures->cnt), 1, fs);
    assert(textures->cnt >= 0 && textures->cnt <= TEXTURE_LIMIT);

    if (textures->cnt > 0) {
        // Reserve scratch space memory for pixel data, to be reused for all textures.
        unsigned char* const px_data = Push(TEXTURE_PX_DATA_SIZE_LIMIT, scratch_space);

        if (!px_data) {
            return false;
        }

        // Load textures.
        GL_CALL(glGenTextures(textures->cnt, textures->gl_ids));

        for (int i = 0; i < textures->cnt; ++i) {
            fread(&textures->sizes[i], sizeof(textures->sizes[i]), 1, fs);

            const int tex_px_data_size = TexturePixelDataSize(textures->sizes[i]);
            fread(px_data, tex_px_data_size, 1, fs);

            SetUpGLTexture(textures->gl_ids[i], textures->sizes[i], px_data);
        }
    }

    // Clear out the memory in the scratch space that we used in this function.
    RewindMemArena(scratch_space, scratch_space_begin_offs);

    return true;
}

static bool LoadFontsFromFS(s_fonts* const fonts, FILE* const fs, s_mem_arena* const scratch_space) {
    assert(fonts);
    assert(IsClear(fonts, sizeof(*fonts)));
    assert(fs);
    assert(IsMemArenaValid(scratch_space));

    // Save the offset in the scratch space arena where we started at, so we can revert back at the end of the function.
    const int scratch_space_begin_offs = scratch_space->offs;

    // Read and verify font count.
    fread(&fonts->cnt, sizeof(fonts->cnt), 1, fs);
    assert(fonts->cnt >= 0 && fonts->cnt <= FONT_LIMIT);

    if (fonts->cnt > 0) {
        // Reserve scratch space memory for pixel data, to be reused for all font textures.
        unsigned char* const px_data = Push(TEXTURE_PX_DATA_SIZE_LIMIT, scratch_space);

        if (!px_data) {
            return false;
        }

        // Load fonts.
        GL_CALL(glGenTextures(fonts->cnt, fonts->tex_gl_ids));

        for (int i = 0; i < fonts->cnt; ++i) {
            fread(&fonts->arrangement_infos[i], sizeof(fonts->arrangement_infos[i]), 1, fs);
            fread(&fonts->tex_sizes[i], sizeof(fonts->tex_sizes[i]), 1, fs);
            fread(px_data, TEXTURE_PX_DATA_SIZE_LIMIT, 1, fs);
            SetUpGLTexture(fonts->tex_gl_ids[i], fonts->tex_sizes[i], px_data);
        }
    }

    // Clear out the memory in the scratch space that we used in this function.
    RewindMemArena(scratch_space, scratch_space_begin_offs);

    return true;
}

static bool LoadShaderProgsFromFS(s_shader_progs* const progs, FILE* const fs, s_mem_arena* const scratch_space) {
    assert(progs);
    assert(IsClear(progs, sizeof(*progs)));
    assert(fs);
    assert(IsMemArenaValid(scratch_space));

    // Save the offset in the scratch space arena where we started at, so we can revert back at the end of the function.
    const int scratch_space_begin_offs = scratch_space->offs;

    // Read and verify the shader program count.
    fread(&progs->cnt, sizeof(progs->cnt), 1, fs);
    assert(progs->cnt >= 0 && progs->cnt <= SHADER_PROG_LIMIT);

    if (progs->cnt > 0) {
        // Reserve memory to store shader source code.
        char* const vert_shader_src_buf = Push(SHADER_SRC_LEN_LIMIT + 1, scratch_space);

        if (IsClear(&vert_shader_src_buf, sizeof(vert_shader_src_buf))) {
            return false;
        }

        char* const frag_shader_src_buf = Push(SHADER_SRC_LEN_LIMIT + 1, scratch_space);

        if (IsClear(&frag_shader_src_buf, sizeof(frag_shader_src_buf))) {
            return false;
        }

        for (int i = 0; i < progs->cnt; ++i) {
            // Get vertex shader source.
            int vert_shader_src_len;
            fread(&vert_shader_src_len, sizeof(vert_shader_src_len), 1, fs);
            fread(vert_shader_src_buf, vert_shader_src_len, 1, fs);
            vert_shader_src_buf[vert_shader_src_len] = '\0';

            // Get fragment shader source.
            int frag_shader_src_len;
            fread(&frag_shader_src_len, sizeof(frag_shader_src_len), 1, fs);
            fread(frag_shader_src_buf, frag_shader_src_len, 1, fs);
            frag_shader_src_buf[frag_shader_src_len] = '\0';

            // Create the program.
            progs->gl_ids[i] = CreateShaderProgFromSrcs(vert_shader_src_buf, frag_shader_src_buf);

            if (!progs->gl_ids[i]) {
                return false;
            }
        }
    }

    return true;
}

static bool LoadSoundsFromFS(s_sounds* const snds, FILE* const fs, s_mem_arena* const scratch_space) {
    assert(snds);
    assert(IsClear(snds, sizeof(*snds)));
    assert(fs);
    assert(scratch_space);

    // Save the offset in the scratch space arena where we started at, so we can revert back at the end of the function.
    const int scratch_space_begin_offs = scratch_space->offs;

    // Read and verify sound count.
    fread(&snds->cnt, sizeof(snds->cnt), 1, fs);
    assert(snds->cnt >= 0 && snds->cnt <= SOUND_LIMIT);

    if (snds->cnt > 0) {
        alGenBuffers(snds->cnt, snds->buf_al_ids);

        ta_audio_sample* const samples = PushAligned(sizeof(*samples) * SOUND_SAMPLE_LIMIT, alignof(ta_audio_sample), scratch_space);

        for (int i = 0; i < snds->cnt; ++i) {
            s_audio_info audio_info;
            fread(&audio_info, sizeof(audio_info), 1, fs);

            const long long sample_cnt = audio_info.sample_cnt_per_channel * audio_info.channel_cnt;
            fread(samples, sizeof(ta_audio_sample), sample_cnt, fs);

            const ALenum format = audio_info.channel_cnt == 1 ? AL_FORMAT_MONO_FLOAT32 : AL_FORMAT_STEREO_FLOAT32;
            alBufferData(snds->buf_al_ids[i], format, samples, sizeof(ta_audio_sample) * sample_cnt, audio_info.sample_rate);
        }
    }

    return true;
}

static bool LoadMusicFromFS(s_music* const music, FILE* const fs) {
    assert(music);
    assert(IsClear(music, sizeof(*music)));
    assert(fs);

    fread(&music->cnt, sizeof(music->cnt), 1, fs);
    assert(music->cnt >= 0 && music->cnt <= MUSIC_LIMIT);

    for (int i = 0; i < music->cnt; ++i) {
        fread(&music->infos[i], sizeof(s_audio_info), 1, fs);
        music->sample_data_file_positions[i] = ftell(fs);
    }

    return true;
}

s_assets* LoadAssets(s_mem_arena* const mem_arena, s_mem_arena* const scratch_space) {
    assert(mem_arena);
    assert(scratch_space);

    // Push the asset data to the memory arena.
    s_assets* const assets = PushAligned(sizeof(*assets), alignof(s_assets), mem_arena);

    if (!assets) {
        LogError("Failed to reserve memory for assets!");
        return NULL;
    }

    // Open the assets file.
    FILE* const fs = fopen(ASSETS_FILE_NAME, "rb");

    if (!fs) {
        LogError("Failed to open \"%s\"!", ASSETS_FILE_NAME);
        return NULL;
    }

    // Load the assets of each type using the file.
    bool success = false;

    do {
        if (!LoadTexturesFromFS(&assets->textures, fs, scratch_space)) {
            LogError("Failed to load textures!");
            break;
        }

        if (!LoadFontsFromFS(&assets->fonts, fs, scratch_space)) {
            LogError("Failed to load fonts!");
            break;
        }

        if (!LoadShaderProgsFromFS(&assets->shader_progs, fs, scratch_space)) {
            LogError("Failed to load shader programs!");
            break;
        }

        if (!LoadSoundsFromFS(&assets->sounds, fs, scratch_space)) {
            LogError("Failed to load sounds!");
            break;
        }

        if (!LoadMusicFromFS(&assets->music, fs)) {
            LogError("Failed to load music!");
            break;
        }

        success = true;
    } while (false);

    fclose(fs);

    return success ? assets : NULL;
}

void UnloadAssets(s_assets* const assets) {
    assert(assets);

    if (assets->sounds.cnt > 0) {
        alDeleteBuffers(assets->sounds.cnt, assets->sounds.buf_al_ids);
    }

    for (int i = 0; i < assets->shader_progs.cnt; ++i) {
        GL_CALL(glDeleteProgram(assets->shader_progs.gl_ids[i]));
    }

    if (assets->fonts.cnt > 0) {
        GL_CALL(glDeleteTextures(assets->fonts.cnt, assets->fonts.tex_gl_ids));
    }

    if (assets->textures.cnt > 0) {
        GL_CALL(glDeleteTextures(assets->textures.cnt, assets->textures.gl_ids));
    }

    Clear(assets, sizeof(*assets));
}

GLuint CreateShaderFromSrc(const char* const src, const bool frag) {
    const GLuint gl_id = GL_CALL(glCreateShader(frag ? GL_FRAGMENT_SHADER : GL_VERTEX_SHADER));
    GL_CALL(glShaderSource(gl_id, 1, &src, NULL));
    GL_CALL(glCompileShader(gl_id));

    GLint compile_success;
    GL_CALL(glGetShaderiv(gl_id, GL_COMPILE_STATUS, &compile_success));

    if (!compile_success) {
        GL_CALL(glDeleteShader(gl_id));
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
