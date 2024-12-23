#include <zf4_assets.h>

#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>
#include <AL/alext.h>

static void set_up_gl_tex(const GLuint glID, const ZF4Pt2D size, const unsigned char* const pxData) {
    glBindTexture(GL_TEXTURE_2D, glID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pxData);
}

static bool load_textures(ZF4Textures* const textures, ZF4MemArena* const memArena, FILE* const fs) {
    fread(&textures->cnt, sizeof(textures->cnt), 1, fs);

    if (textures->cnt > 0) {
        // Reserve space in the arena for texture data.
        textures->glIDs = zf4_push_to_mem_arena(memArena, sizeof(*textures->glIDs) * textures->cnt, alignof(GLuint));

        if (!textures->glIDs) {
            return false;
        }

        textures->sizes = zf4_push_to_mem_arena(memArena, sizeof(*textures->sizes) * textures->cnt, alignof(ZF4Vec2D));

        if (!textures->sizes) {
            return false;
        }

        // Allocate memory for pixel data (reused for all textures).
        unsigned char* const pxData = malloc(ZF4_TEX_PX_DATA_SIZE_LIMIT);

        if (!pxData) {
            return false;
        }

        // Load textures.
        if (textures->cnt > 0) {
            glGenTextures(textures->cnt, textures->glIDs);

            for (int i = 0; i < textures->cnt; ++i) {
                fread(&textures->sizes[i], sizeof(textures->sizes[i]), 1, fs);
                fread(pxData, ZF4_TEX_CHANNEL_CNT * textures->sizes[i].x * textures->sizes[i].y, 1, fs);
                set_up_gl_tex(textures->glIDs[i], textures->sizes[i], pxData);
            }
        }

        free(pxData);
    }

    return true;
}

static bool load_fonts(ZF4Fonts* const fonts, ZF4MemArena* const memArena, FILE* const fs) {
    fread(&fonts->cnt, sizeof(fonts->cnt), 1, fs);

    if (fonts->cnt > 0) {
        // Reserve space in the arena for font data.
        fonts->arrangementInfos = zf4_push_to_mem_arena(memArena, sizeof(*fonts->arrangementInfos) * fonts->cnt, alignof(ZF4FontArrangementInfo));

        if (!fonts->arrangementInfos) {
            return false;
        }

        fonts->texGLIDs = zf4_push_to_mem_arena(memArena, sizeof(*fonts->texGLIDs) * fonts->cnt, alignof(GLuint));

        if (!fonts->texGLIDs) {
            return false;
        }

        fonts->texSizes = zf4_push_to_mem_arena(memArena, sizeof(*fonts->texSizes) * fonts->cnt, alignof(ZF4Pt2D));

        if (!fonts->texSizes) {
            return false;
        }

        // Allocate memory for pixel data, to be reused for all font textures.
        unsigned char* const pxData = malloc(ZF4_TEX_PX_DATA_SIZE_LIMIT);

        if (!pxData) {
            return false;
        }

        // Load fonts.
        glGenTextures(fonts->cnt, fonts->texGLIDs);

        for (int i = 0; i < fonts->cnt; ++i) {
            fread(&fonts->arrangementInfos[i], sizeof(fonts->arrangementInfos[i]), 1, fs);
            fread(&fonts->texSizes[i], sizeof(fonts->texSizes[i]), 1, fs);
            fread(pxData, ZF4_TEX_PX_DATA_SIZE_LIMIT, 1, fs);
            set_up_gl_tex(fonts->texGLIDs[i], fonts->texSizes[i], pxData);
        }

        free(pxData);
    }

    return true;
}

static bool load_sounds(ZF4Sounds* const snds, ZF4MemArena* const memArena, FILE* const fs) {
    fread(&snds->cnt, sizeof(snds->cnt), 1, fs);

    if (snds->cnt > 0) {
        snds->bufALIDs = zf4_push_to_mem_arena(memArena, sizeof(*snds->bufALIDs) * snds->cnt, alignof(ALuint));

        if (!snds->bufALIDs) {
            return false;
        }

        float* const samples = malloc(sizeof(*samples) * ZF4_SOUND_SAMPLE_LIMIT);

        if (!samples) {
            return false;
        }

        alGenBuffers(snds->cnt, snds->bufALIDs);

        for (int i = 0; i < snds->cnt; ++i) {
            ZF4AudioInfo audioInfo;
            fread(&audioInfo, sizeof(audioInfo), 1, fs);

            const int sampleCnt = audioInfo.sampleCntPerChannel * audioInfo.channelCnt;
            fread(samples, sizeof(*samples), sampleCnt, fs);

            const ALenum format = audioInfo.channelCnt == 1 ? AL_FORMAT_MONO_FLOAT32 : AL_FORMAT_STEREO_FLOAT32;
            alBufferData(snds->bufALIDs[i], format, samples, sizeof(*samples) * sampleCnt, audioInfo.sampleRate);
        }

        free(samples);
    }

    return true;
}

bool zf4_load_assets(ZF4Assets* const assets, ZF4MemArena* const memArena) {
    assert(zf4_is_zero(assets, sizeof(*assets)));

    FILE* const fs = fopen(ZF4_ASSETS_FILE_NAME, "rb");

    if (!fs) {
        zf4_log_error("Failed to open \"%s\"!", ZF4_ASSETS_FILE_NAME);
        return false;
    }

    if (!load_textures(&assets->textures, memArena, fs)
        || !load_fonts(&assets->fonts, memArena, fs)
        || !load_sounds(&assets->sounds, memArena, fs)) {
        fclose(fs);
        return false;
    }

    fclose(fs);

    return true;
}

void zf4_unload_assets(ZF4Assets* const assets) {
    if (assets->sounds.cnt > 0) {
        alDeleteBuffers(assets->sounds.cnt, assets->sounds.bufALIDs);
    }

    if (assets->fonts.cnt > 0) {
        glDeleteTextures(assets->fonts.cnt, assets->fonts.texGLIDs);
    }

    if (assets->textures.cnt > 0) {
        glDeleteTextures(assets->textures.cnt, assets->textures.glIDs);
    }

    memset(assets, 0, sizeof(*assets));
}
