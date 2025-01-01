#include <zf4_scenes.h>

namespace zf4 {
    bool load_scene(Scene* scene, int sceneTypeIndex) {
        assert(is_zero(scene));

        log("Loading scene of type index %d...", sceneTypeIndex);

        scene->typeIndex = sceneTypeIndex;

        SceneTypeInfo* sceneTypeInfo = get_scene_type_info(sceneTypeIndex);

        if (!scene->memArena.init(sceneTypeInfo->memArenaSize)) {
            log_error("Failed to initialise scene main memory arena!");
            return false;
        }

        if (!scene->scratchSpace.init(gk_sceneScratchSpaceSize)) {
            log_error("Failed to initialise scene scratch space memory arena!");
            return false;
        }

        if (!load_renderer(&scene->renderer, &scene->memArena, sceneTypeInfo->renderLayerCnt, sceneTypeInfo->camRenderLayerCnt, sceneTypeInfo->renderLayerPropsInitializer)) {
            log_error("Failed to load scene renderer!");
            return false;
        }

        if (!scene->entManager.load(sceneTypeInfo->entLimit, sceneTypeInfo->compTypeLimitLoader, &scene->memArena)) {
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

        return sceneTypeInfo->init(scene);
    }

    void unload_scene(Scene* scene) {
        clean_renderer(&scene->renderer);
        scene->scratchSpace.clean();
        scene->memArena.clean();
        zero_out(scene);
    }

    bool proc_scene_tick(Scene* scene) {
        SceneTypeInfo* sceneTypeInfo = get_scene_type_info(scene->typeIndex);

        empty_sprite_batches(&scene->renderer);

        scene->scratchSpace.reset();

        int sceneChangeIndex = -1;

        if (!sceneTypeInfo->tick(scene, &sceneChangeIndex)) {
            return false;
        }

        if (sceneChangeIndex != -1) {
            log("Scene change request detected.");

            unload_scene(scene);

            if (!load_scene(scene, sceneChangeIndex)) {
                return false;
            }
        }

        return true;
    }
}
