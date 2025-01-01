#include <zf4_scenes.h>

namespace zf4 {
    static bool load_scene_ent(Scene* scene, Ent* ent) {
        ent->compIndexes = scene->memArena.push<int>(get_component_type_cnt());

        if (!ent->compIndexes) {
            return false;
        }

        ent->compSig = scene->memArena.push<Byte>(bits_to_bytes(get_component_type_cnt()));

        if (!ent->compSig) {
            return false;
        }

        return true;
    }

    static bool load_scene_ents(Scene* scene) {
        SceneTypeInfo* sceneTypeInfo = get_scene_type_info(scene->typeIndex);

        if (sceneTypeInfo->entLimit > 0) {
            //
            // Entities
            //
            scene->ents = scene->memArena.push<Ent>(sceneTypeInfo->entLimit);

            if (!scene->ents) {
                return false;
            }

            for (int i = 0; i < sceneTypeInfo->entLimit; ++i) {
                if (!load_scene_ent(scene, &scene->ents[i])) {
                    return false;
                }
            }

            scene->entActivity = scene->memArena.push<Byte>(bits_to_bytes(sceneTypeInfo->entLimit));

            if (!scene->entActivity) {
                return false;
            }

            scene->entVersions = scene->memArena.push<int>(sceneTypeInfo->entLimit);

            if (!scene->entVersions) {
                return false;
            }

            //
            // Components
            //
            scene->compArrays = scene->memArena.push<void*>(get_component_type_cnt());

            if (!scene->compArrays) {
                return false;
            }

            scene->compActivities = scene->memArena.push<Byte*>(get_component_type_cnt());

            if (!scene->compActivities) {
                return false;
            }

            scene->compTypeLimits = scene->memArena.push<int>(get_component_type_cnt());

            if (!scene->compTypeLimits) {
                return false;
            }

            for (int i = 0; i < get_component_type_cnt(); ++i) {
                scene->compTypeLimits[i] = sceneTypeInfo->entLimit; // The default component type limit is the entity limit.
                sceneTypeInfo->compTypeLimitLoader(&scene->compTypeLimits[i], i); // The limit may or may not be changed here.

                ComponentTypeInfo* compTypeInfo = get_component_type_info(i);

                scene->compArrays[i] = scene->memArena.push<Byte>(compTypeInfo->size * scene->compTypeLimits[i]);

                if (!scene->compArrays[i]) {
                    return false;
                }

                scene->compActivities[i] = scene->memArena.push<Byte>(bits_to_bytes(scene->compTypeLimits[i]));

                if (!scene->compActivities[i]) {
                    return false;
                }
            }
        }

        return true;
    }

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

        if (!load_scene_ents(scene)) {
            log_error("Failed to load scene ents!");
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
