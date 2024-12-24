#ifndef ZF4_ASSETS_H
#define ZF4_ASSETS_H

#include <stdbool.h>
#include <stdio.h>
#include <glad/glad.h>
#include <AL/al.h>
#include <zf4c.h>

typedef struct {
    GLuint* glIDs;
    ZF4Pt2D* sizes;
    int cnt;
} ZF4Textures;

typedef struct {
    ZF4FontArrangementInfo* arrangementInfos;

    GLuint* texGLIDs;
    ZF4Pt2D* texSizes;

    int cnt;
} ZF4Fonts;

typedef struct {
    GLuint* bufALIDs;
    int cnt;
} ZF4Sounds;

typedef struct {
    ZF4AudioInfo* infos;
    int* sampleDataFilePositions;
    int cnt;
} ZF4Music;

//
// State
//
bool zf4_load_assets();
void zf4_unload_assets();

const ZF4Textures* zf4_get_textures();
const ZF4Fonts* zf4_get_fonts();
const ZF4Sounds* zf4_get_sounds();
const ZF4Music* zf4_get_music();

//
// Logic
//
bool zf4_load_textures(ZF4Textures* const textures, ZF4MemArena* const memArena, FILE* const fs);
bool zf4_load_fonts(ZF4Fonts* const fonts, ZF4MemArena* const memArena, FILE* const fs);
bool zf4_load_sounds(ZF4Sounds* const snds, ZF4MemArena* const memArena, FILE* const fs);
bool zf4_load_music(ZF4Music* const music, ZF4MemArena* const memArena, FILE* const fs);

#endif
