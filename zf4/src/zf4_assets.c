#include <zf4_assets.h>

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

        // Allocate memory for pixel data, to be reused for all textures.
        const int pxDataSize = ZF4_TEX_CHANNEL_CNT * ZF4_TEX_WIDTH_LIMIT * ZF4_TEX_HEIGHT_LIMIT;
        unsigned char* const pxData = malloc(pxDataSize);

        if (!pxData) {
            return false;
        }

        // Load textures.
        if (textures->cnt > 0) {
            glGenTextures(textures->cnt, textures->glIDs);

            for (int i = 0; i < textures->cnt; ++i) {
                fread(&textures->sizes[i], sizeof(textures->sizes[i]), 1, fs);
                fread(pxData, ZF4_TEX_CHANNEL_CNT * textures->sizes[i].x * textures->sizes[i].y, 1, fs);

                glBindTexture(GL_TEXTURE_2D, textures->glIDs[i]);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textures->sizes[i].x, textures->sizes[i].y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pxData);
            }
        }

        free(pxData);
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

    if (!load_textures(&assets->textures, memArena, fs)) {
        fclose(fs);
        return false;
    }

    fclose(fs);

    return true;
}

void zf4_unload_assets(ZF4Assets* const assets) {
    if (assets->textures.cnt > 0) {
        glDeleteTextures(assets->textures.cnt, assets->textures.glIDs);
    }

    memset(assets, 0, sizeof(*assets));
}
