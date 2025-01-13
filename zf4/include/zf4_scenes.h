#pragma once

#include <zf4c.h>
#include <zf4_ecs.h>

namespace zf4 {
    constexpr int gk_sceneScratchSpaceSize = megabytes_to_bytes(2);

    struct Scene {
        int typeIndex;

        MemArena memArena;
        MemArena scratchSpace;

        EntityManager entManager;

        void* userData;
    };

    struct GamePtrs;

    using SceneInit = bool (*)(Scene* const scene, const GamePtrs& gamePtrs);
    using SceneTick = bool (*)(Scene* const scene, int* const sceneChangeIndex, const GamePtrs& gamePtrs);

    struct SceneType {
        int memArenaSize;

        int entLimit;

        SceneInit init;
        SceneTick tick;

        int userDataSize;
        int userDataAlignment;
    };

    using SceneTypeInitializer = void (*)(SceneType* const type, const int index);

    bool load_scene(Scene* const scene, const int typeIndex, const GamePtrs& gamePtrs);
    void unload_scene(Scene* const scene);
    bool proc_scene_tick(Scene* const scene, const GamePtrs& gamePtrs);
}
