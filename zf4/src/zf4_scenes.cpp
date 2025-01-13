#include <zf4_scenes.h>

#include <zf4_game.h>

namespace zf4 {
    bool load_scene(Scene* const scene, const int sceneTypeIndex, const GamePtrs& gamePtrs) {
        assert(is_zero(scene));

        log("Loading scene of type index %d...", sceneTypeIndex);

        scene->typeIndex = sceneTypeIndex;

        const SceneType& sceneType = gamePtrs.sceneTypes[sceneTypeIndex];

        if (!scene->memArena.init(sceneType.memArenaSize)) {
            log_error("Failed to initialise scene main memory arena!");
            return false;
        }

        if (!scene->scratchSpace.init(gk_sceneScratchSpaceSize)) {
            log_error("Failed to initialise scene scratch space memory arena!");
            return false;
        }

        if (!scene->entManager.load(&scene->memArena, sceneType.entLimit, gamePtrs.compTypes)) {
            log_error("Failed to load the scene entity manager!");
            return false;
        }

        if (sceneType.userDataSize > 0) {
            scene->userData = scene->memArena.push(sceneType.userDataSize, sceneType.userDataAlignment);

            if (!scene->userData) {
                log_error("Failed to reserve memory for scene user data!");
                return false;
            }
        }

        return sceneType.init(scene, gamePtrs);
    }

    void unload_scene(Scene* const scene) {
        scene->scratchSpace.clean();
        scene->memArena.clean();
        zero_out(scene);
    }

    bool proc_scene_tick(Scene* const scene, const GamePtrs& gamePtrs) {
        scene->scratchSpace.reset();

        gamePtrs.renderer->begin_submission_phase();

        int sceneChangeIndex = -1;

        if (!gamePtrs.sceneTypes[scene->typeIndex].tick(scene, &sceneChangeIndex, gamePtrs)) {
            return false;
        }

        gamePtrs.renderer->end_submission_phase();

        if (sceneChangeIndex != -1) {
            log("Scene change request detected.");

            unload_scene(scene);

            if (!load_scene(scene, sceneChangeIndex, gamePtrs)) {
                return false;
            }
        }

        return true;
    }
}
