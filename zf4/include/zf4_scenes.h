#pragma once

#include <zf4c.h>
#include <zf4_renderer.h>
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

    struct GamePtrs {
        Renderer* renderer;
    };

    using SceneInit = bool (*)(Scene* const scene, const GamePtrs& gamePtrs);
    using SceneTick = bool (*)(Scene* const scene, int* const sceneChangeIndex, const GamePtrs& gamePtrs);

    struct SceneTypeInfo {
        int memArenaSize;

        Vec3D bgColor;

        int entLimit;
        ComponentTypeLimitLoader compTypeLimitLoader;

        SceneInit init;
        SceneTick tick;

        int userDataSize;
        int userDataAlignment;
    };

    using SceneTypeInfosLoader = bool (*)(Array<SceneTypeInfo>* const typeInfos, MemArena* const memArena);

    bool load_scene(Scene* const scene, const int typeIndex, const Array<SceneTypeInfo>& sceneTypeInfos, const GamePtrs& gamePtrs);
    void unload_scene(Scene* const scene);
    bool proc_scene_tick(Scene* const scene, const Array<SceneTypeInfo>& sceneTypeInfos, const GamePtrs& gamePtrs);
}
