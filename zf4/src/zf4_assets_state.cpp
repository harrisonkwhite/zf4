#include <zf4_assets.h>

namespace zf4 {
    static constexpr int ik_memArenaSize = megabytes_to_bytes(2);

    struct Assets {
        MemArena memArena;

        Textures textures;
        Fonts fonts;
        Sounds sounds;
        Music music;
    };

    static Assets i_assets;

    bool load_assets() {
        assert(is_zero(&i_assets));

        if (!i_assets.memArena.init(ik_memArenaSize)) {
            log_error("Failed to initialise the assets memory arena!");
            return false;
        }

        FILE* const fs = fopen(gk_assetsFileName, "rb");

        if (!fs) {
            log_error("Failed to open \"%s\"!", gk_assetsFileName);
            return false;
        }

        if (!load_textures(&i_assets.textures, &i_assets.memArena, fs)) {
            log_error("Failed to load textures!");
            fclose(fs);
            return false;
        }

        if (!load_fonts(&i_assets.fonts, &i_assets.memArena, fs)) {
            log_error("Failed to load fonts!");
            fclose(fs);
            return false;
        }

        if (!load_sounds(&i_assets.sounds, &i_assets.memArena, fs)) {
            log_error("Failed to load sounds!");
            fclose(fs);
            return false;
        }

        if (!load_music(&i_assets.music, &i_assets.memArena, fs)) {
            log_error("Failed to load music!");
            fclose(fs);
            return false;
        }

        fclose(fs);

        return true;
    }

    void unload_assets() {
        if (i_assets.sounds.cnt > 0) {
            alDeleteBuffers(i_assets.sounds.cnt, i_assets.sounds.bufALIDs);
        }

        if (i_assets.fonts.cnt > 0) {
            glDeleteTextures(i_assets.fonts.cnt, i_assets.fonts.texGLIDs);
        }

        if (i_assets.textures.cnt > 0) {
            glDeleteTextures(i_assets.textures.cnt, i_assets.textures.glIDs);
        }

        i_assets.memArena.clean();

        zero_out(&i_assets);
    }

    const Textures* get_textures() {
        return &i_assets.textures;
    }

    const Fonts* get_fonts() {
        return &i_assets.fonts;
    }

    const Sounds* get_sounds() {
        return &i_assets.sounds;
    }

    const Music* get_music() {
        return &i_assets.music;
    }
}
