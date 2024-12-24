#ifndef ZF4_ASSETS_H
#define ZF4_ASSETS_H

#include <stdbool.h>
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

typedef struct {
    ZF4Textures textures;
    ZF4Fonts fonts;
    ZF4Sounds sounds;
    ZF4Music music;
} ZF4Assets;

bool zf4_load_assets(ZF4Assets* const assets, ZF4MemArena* const memArena);
void zf4_unload_assets(ZF4Assets* const assets);

#endif
