#pragma once

#include <zf4_sprites.h>
#include <zf4_scenes.h>

namespace zf4 {
    struct UserGameInfo {
        int windowInitWidth;
        int windowInitHeight;
        const char* windowTitle;
        bool windowResizable;
        bool windowHideCursor;

        int spriteCnt;
        SpriteLoader spriteLoader;

        int componentTypeCnt;
        ComponentTypeInfoLoader componentTypeInfoLoader;

        int sceneTypeCnt;
        SceneTypeInfoLoader sceneTypeInfoLoader;
    };

    void start_game(const UserGameInfo* const userInfo);
}
