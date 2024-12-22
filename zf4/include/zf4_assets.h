#ifndef ZF4_ASSETS_H
#define ZF4_ASSETS_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdalign.h>
#include <glad/glad.h>
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
    ZF4Textures textures;
    ZF4Fonts fonts;
} ZF4Assets;

bool zf4_load_assets(ZF4Assets* const assets, ZF4MemArena* const memArena);
void zf4_unload_assets(ZF4Assets* const assets);

#endif
