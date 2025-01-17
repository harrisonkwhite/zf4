#pragma once

#include <zf4_window.h>
#include <zf4_assets.h>
#include <zf4_rendering.h>
#include <zf4_audio.h>
#include <zf4_sprites.h>
#include <zf4_ecs.h>

namespace zf4 {
    constexpr int gk_glVersionMajor = 4;
    constexpr int gk_glVersionMinor = 3;

    struct GamePtrs {
        MemArena& permMemArena;
        MemArena& tempMemArena;
        const Window& window;
        const Assets& assets;
        Renderer& renderer;
        SoundSrcManager& soundSrcManager;
        MusicSrcManager& musicSrcManager;
        const zf4::Array<Sprite>& sprites;
        const zf4::Array<ComponentType>& compTypes;
        EntityManager& entManager;
    };

    using GameInit = bool (*)(const GamePtrs& gamePtrs);
    using GameTick = bool (*)(const GamePtrs& gamePtrs);
    using GameDraw = bool (*)(const GamePtrs& gamePtrs);
    using GameCleanup = void (*)();

    struct GameInfo {
        GameInit init;
        GameTick tick;
        GameDraw draw;
        GameCleanup cleanup;

        int permMemArenaSize;
        int tempMemArenaSize;

        const Vec2DI windowInitSize;
        const char* windowTitle;
        WindowFlags windowFlags;

        int renderBatchLimit;
        int renderBatchLife;

        int spriteCnt;
        SpriteInitializer spriteInitializer;

        int componentTypeCnt;
        ComponentTypeInitializer componentTypeInitializer;

        int entLimit;
    };

    using GameInfoInitializer = void (*)(GameInfo& info);

    bool run_game(const GameInfoInitializer gameInfoInitializer);
}
