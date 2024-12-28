#ifndef ZF4_GAME_H
#define ZF4_GAME_H

#include <zf4_sprites.h>
#include <zf4_scenes.h>

typedef struct {
    int windowInitWidth;
    int windowInitHeight;
    const char* windowTitle;
    bool windowResizable;
    bool windowHideCursor;

    int spriteCnt;
    ZF4SpriteLoader spriteLoader;

    int componentTypeCnt;
    ZF4ComponentTypeInfoLoader componentTypeInfoLoader;

    int sceneTypeCnt;
    ZF4SceneTypeInfoLoader sceneTypeInfoLoader;
} ZF4UserGameInfo;

void zf4_start_game(const ZF4UserGameInfo* const userInfo);

#endif
