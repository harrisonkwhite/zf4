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

        SpritesLoader spritesLoader;

        int componentTypeCnt;
        ComponentTypeInfoLoader componentTypeInfoLoader;

        SceneTypeInfosLoader sceneTypeInfosLoader;
    };

    void start_game(const UserGameInfo* const userInfo);
    const Array<Sprite>& get_game_sprites();
}
