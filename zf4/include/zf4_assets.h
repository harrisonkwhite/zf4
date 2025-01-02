#pragma once

#include <cstdio>
#include <glad/glad.h>
#include <AL/al.h>
#include <zf4c.h>

namespace zf4 {
    struct Textures {
        GLuint* glIDs;
        Vec2DI* sizes;
        int cnt;
    };

    struct Fonts {
        FontArrangementInfo* arrangementInfos;

        GLuint* texGLIDs;
        Vec2DI* texSizes;

        int cnt;
    };

    struct Sounds {
        GLuint* bufALIDs;
        int cnt;
    };

    struct Music {
        AudioInfo* infos;
        int* sampleDataFilePositions;
        int cnt;
    };

    //
    // State
    //
    bool load_assets();
    void unload_assets();

    const Textures* get_textures();
    const Fonts* get_fonts();
    const Sounds* get_sounds();
    const Music* get_music();

    //
    // Logic
    //
    bool load_textures(Textures* const textures, MemArena* const memArena, FILE* const fs);
    bool load_fonts(Fonts* const fonts, MemArena* const memArena, FILE* const fs);
    bool load_sounds(Sounds* const snds, MemArena* const memArena, FILE* const fs);
    bool load_music(Music* const music, MemArena* const memArena, FILE* const fs);
}
