#include <zf4_scenes.h>

namespace zf4 {
    bool load_scene(Scene* const scene, const int sceneTypeIndex, const Array<SceneTypeInfo>& sceneTypeInfos, const GamePtrs& gamePtrs) {
        assert(is_zero(scene));

        log("Loading scene of type index %d...", sceneTypeIndex);

        scene->typeIndex = sceneTypeIndex;

        const SceneTypeInfo* const sceneTypeInfo = &sceneTypeInfos[sceneTypeIndex];

        if (!scene->memArena.init(sceneTypeInfo->memArenaSize)) {
            log_error("Failed to initialise scene main memory arena!");
            return false;
        }

        if (!scene->scratchSpace.init(gk_sceneScratchSpaceSize)) {
            log_error("Failed to initialise scene scratch space memory arena!");
            return false;
        }

        if (!scene->renderer.init(&scene->memArena, sceneTypeInfo->renderSurfCnt, sceneTypeInfo->renderBatchCnt)) {
            log_error("Failed to initialise the scene renderer!");
            return false;
        }

        if (!scene->entManager.load(&scene->memArena, sceneTypeInfo->entLimit, sceneTypeInfo->compTypeLimitLoader)) {
            log_error("Failed to load the scene entity manager!");
            return false;
        }

        if (sceneTypeInfo->userDataSize > 0) {
            scene->userData = scene->memArena.push(sceneTypeInfo->userDataSize, sceneTypeInfo->userDataAlignment);

            if (!scene->userData) {
                log_error("Failed to reserve memory for scene user data!");
                return false;
            }
        }

        return sceneTypeInfo->init(scene, gamePtrs);
    }

    void unload_scene(Scene* const scene) {
        scene->renderer.clean();
        scene->scratchSpace.clean();
        scene->memArena.clean();
        zero_out(scene);
    }

    bool proc_scene_tick(Scene* const scene, const Array<SceneTypeInfo>& sceneTypeInfos, const GamePtrs& gamePtrs) {
        scene->scratchSpace.reset();

        scene->renderer.begin_draw();

        int sceneChangeIndex = -1;

        if (!sceneTypeInfos[scene->typeIndex].tick(scene, &sceneChangeIndex, gamePtrs)) {
            return false;
        }

        scene->renderer.end_draw();

        if (sceneChangeIndex != -1) {
            log("Scene change request detected.");

            unload_scene(scene);

            if (!load_scene(scene, sceneChangeIndex, sceneTypeInfos, gamePtrs)) {
                return false;
            }
        }

        return true;
    }
}
