#include <zf4_assets.h>

#include <cstdalign>
#include <cstdlib>
#include <AL/alext.h>

// TODO: Check for file read errors.

static void set_up_gl_tex(const GLuint glID, const ZF4Pt2D size, const unsigned char* const pxData) {
    glBindTexture(GL_TEXTURE_2D, glID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pxData);
}

bool zf4_load_textures(ZF4Textures* const textures, ZF4MemArena* const memArena, FILE* const fs) {
    zf4_read_from_fs<int>(&textures->cnt, fs);

    if (textures->cnt > 0) {
        // Reserve space in the arena for texture data.
        textures->glIDs = memArena->push<GLuint>(textures->cnt);

        if (!textures->glIDs) {
            return false;
        }

        textures->sizes = memArena->push<ZF4Pt2D>(textures->cnt);

        if (!textures->sizes) {
            return false;
        }

        // Allocate memory for pixel data (reused for all textures).
        const auto pxData = zf4_alloc<unsigned char>(ZF4_TEX_PX_DATA_SIZE_LIMIT);

        if (!pxData) {
            return false;
        }

        // Load textures.
        if (textures->cnt > 0) {
            glGenTextures(textures->cnt, textures->glIDs);

            for (int i = 0; i < textures->cnt; ++i) {
                zf4_read_from_fs<ZF4Pt2D>(&textures->sizes[i], fs);
                zf4_read_from_fs<unsigned char>(pxData, fs, ZF4_TEX_CHANNEL_CNT * textures->sizes[i].x * textures->sizes[i].y);
                set_up_gl_tex(textures->glIDs[i], textures->sizes[i], pxData);
            }
        }

        free(pxData);
    }

    return true;
}

bool zf4_load_fonts(ZF4Fonts* const fonts, ZF4MemArena* const memArena, FILE* const fs) {
    zf4_read_from_fs<int>(&fonts->cnt, fs);

    if (fonts->cnt > 0) {
        // Reserve space in the arena for font data.
        fonts->arrangementInfos = memArena->push<ZF4FontArrangementInfo>(fonts->cnt);

        if (!fonts->arrangementInfos) {
            return false;
        }

        fonts->texGLIDs = memArena->push<GLuint>(fonts->cnt);

        if (!fonts->texGLIDs) {
            return false;
        }

        fonts->texSizes = memArena->push<ZF4Pt2D>(fonts->cnt);

        if (!fonts->texSizes) {
            return false;
        }

        // Allocate memory for pixel data, to be reused for all font textures.
        const auto pxData = zf4_alloc<unsigned char>(ZF4_TEX_PX_DATA_SIZE_LIMIT);

        if (!pxData) {
            return false;
        }

        // Load fonts.
        glGenTextures(fonts->cnt, fonts->texGLIDs);

        for (int i = 0; i < fonts->cnt; ++i) {
            zf4_read_from_fs<ZF4FontArrangementInfo>(&fonts->arrangementInfos[i], fs);
            zf4_read_from_fs<ZF4Pt2D>(&fonts->texSizes[i], fs);
            zf4_read_from_fs<unsigned char>(pxData, fs, ZF4_TEX_PX_DATA_SIZE_LIMIT);
            set_up_gl_tex(fonts->texGLIDs[i], fonts->texSizes[i], pxData);
        }

        free(pxData);
    }

    return true;
}

bool zf4_load_sounds(ZF4Sounds* const snds, ZF4MemArena* const memArena, FILE* const fs) {
    zf4_read_from_fs<int>(&snds->cnt, fs);

    if (snds->cnt > 0) {
        snds->bufALIDs = memArena->push<ALuint>(snds->cnt);

        if (!snds->bufALIDs) {
            return false;
        }

        const auto samples = zf4_alloc<float>(ZF4_SOUND_SAMPLE_LIMIT);

        if (!samples) {
            return false;
        }

        alGenBuffers(snds->cnt, snds->bufALIDs);

        for (int i = 0; i < snds->cnt; ++i) {
            ZF4AudioInfo audioInfo;
            zf4_read_from_fs<ZF4AudioInfo>(&audioInfo, fs);

            const long long sampleCnt = audioInfo.sampleCntPerChannel * audioInfo.channelCnt;
            zf4_read_from_fs<float>(samples, fs, sampleCnt);

            const ALenum format = audioInfo.channelCnt == 1 ? AL_FORMAT_MONO_FLOAT32 : AL_FORMAT_STEREO_FLOAT32;
            alBufferData(snds->bufALIDs[i], format, samples, sizeof(*samples) * sampleCnt, audioInfo.sampleRate);
        }

        free(samples);
    }

    return true;
}

bool zf4_load_music(ZF4Music* const music, ZF4MemArena* const memArena, FILE* const fs) {
    zf4_read_from_fs<int>(&music->cnt, fs);

    if (music->cnt > 0) {
        music->infos = memArena->push<ZF4AudioInfo>(music->cnt);

        if (!music->infos) {
            return false;
        }

        music->sampleDataFilePositions = memArena->push<int>(music->cnt);

        if (!music->sampleDataFilePositions) {
            return false;
        }

        for (int i = 0; i < music->cnt; ++i) {
            zf4_read_from_fs<ZF4AudioInfo>(&music->infos[i], fs);
            music->sampleDataFilePositions[i] = ftell(fs);
        }
    }

    return true;
}
