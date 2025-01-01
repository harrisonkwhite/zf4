#pragma once

#include <cstdbool>
#include <cstdio>
#include <glad/glad.h>
#include <AL/al.h>
#include <zf4c.h>

namespace zf4 {
    typedef struct {
        GLuint* glIDs;
        Pt2D* sizes;
        int cnt;
    } Textures;

    typedef struct {
        FontArrangementInfo* arrangementInfos;

        GLuint* texGLIDs;
        Pt2D* texSizes;

        int cnt;
    } Fonts;

    typedef struct {
        GLuint* bufALIDs;
        int cnt;
    } Sounds;

    typedef struct {
        AudioInfo* infos;
        int* sampleDataFilePositions;
        int cnt;
    } Music;

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
