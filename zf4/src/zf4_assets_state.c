#include <zf4_assets.h>

#define MEM_ARENA_SIZE ZF4_MEGABYTES(2)

typedef struct {
    ZF4MemArena memArena;

    ZF4Textures textures;
    ZF4Fonts fonts;
    ZF4Sounds sounds;
    ZF4Music music;
} ZF4Assets;

static ZF4Assets i_assets;

bool zf4_load_assets() {
    assert(zf4_is_zero(&i_assets, sizeof(i_assets)));

    if (!zf4_init_mem_arena(&i_assets.memArena, MEM_ARENA_SIZE)) {
        return false;
    }

    FILE* const fs = fopen(ZF4_ASSETS_FILE_NAME, "rb");

    if (!fs) {
        zf4_log_error("Failed to open \"%s\"!", ZF4_ASSETS_FILE_NAME);
        return false;
    }

    if (!zf4_load_textures(&i_assets.textures, &i_assets.memArena, fs)) {
        zf4_log_error("Failed to load textures!");
        fclose(fs);
        return false;
    }

    if (!zf4_load_fonts(&i_assets.fonts, &i_assets.memArena, fs)) {
        zf4_log_error("Failed to load fonts!");
        fclose(fs);
        return false;
    }

    if (!zf4_load_sounds(&i_assets.sounds, &i_assets.memArena, fs)) {
        zf4_log_error("Failed to load sounds!");
        fclose(fs);
        return false;
    }

    if (!zf4_load_music(&i_assets.music, &i_assets.memArena, fs)) {
        zf4_log_error("Failed to load music!");
        fclose(fs);
        return false;
    }

    fclose(fs);

    return true;
}

void zf4_unload_assets() {
    if (i_assets.sounds.cnt > 0) {
        alDeleteBuffers(i_assets.sounds.cnt, i_assets.sounds.bufALIDs);
    }

    if (i_assets.fonts.cnt > 0) {
        glDeleteTextures(i_assets.fonts.cnt, i_assets.fonts.texGLIDs);
    }

    if (i_assets.textures.cnt > 0) {
        glDeleteTextures(i_assets.textures.cnt, i_assets.textures.glIDs);
    }

    zf4_clean_mem_arena(&i_assets.memArena);

    memset(&i_assets, 0, sizeof(i_assets));
}

const ZF4Textures* zf4_get_textures() {
    return &i_assets.textures;
}

const ZF4Fonts* zf4_get_fonts() {
    return &i_assets.fonts;
}

const ZF4Sounds* zf4_get_sounds() {
    return &i_assets.sounds;
}

const ZF4Music* zf4_get_music() {
    return &i_assets.music;
}
