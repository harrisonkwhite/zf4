#include <zf4_scenes.h>

#include <stdalign.h>

static bool load_scene_ent(ZF4Scene* scene, ZF4Ent* ent) {
    ent->compIndexes = scene->memArena.push<int>(zf4_get_component_type_cnt());

    if (!ent->compIndexes) {
        return false;
    }

    ent->compSig = scene->memArena.push<ZF4Byte>(ZF4_BITS_TO_BYTES(zf4_get_component_type_cnt()));

    if (!ent->compSig) {
        return false;
    }

    return true;
}

static bool load_scene_ents(ZF4Scene* scene) {
    ZF4SceneTypeInfo* sceneTypeInfo = zf4_get_scene_type_info(scene->typeIndex);

    if (sceneTypeInfo->entLimit > 0) {
        //
        // Entities
        //
        scene->ents = scene->memArena.push<ZF4Ent>(sceneTypeInfo->entLimit);

        if (!scene->ents) {
            return false;
        }

        for (int i = 0; i < sceneTypeInfo->entLimit; ++i) {
            if (!load_scene_ent(scene, &scene->ents[i])) {
                return false;
            }
        }

        scene->entActivity = scene->memArena.push<ZF4Byte>(ZF4_BITS_TO_BYTES(sceneTypeInfo->entLimit));

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
        scene->compArrays = scene->memArena.push<void*>(zf4_get_component_type_cnt());

        if (!scene->compArrays) {
            return false;
        }

        scene->compActivities = scene->memArena.push<ZF4Byte*>(zf4_get_component_type_cnt());

        if (!scene->compActivities) {
            return false;
        }

        scene->compTypeLimits = scene->memArena.push<int>(zf4_get_component_type_cnt());

        if (!scene->compTypeLimits) {
            return false;
        }

        for (int i = 0; i < zf4_get_component_type_cnt(); ++i) {
            scene->compTypeLimits[i] = sceneTypeInfo->entLimit; // The default component type limit is the entity limit.
            sceneTypeInfo->compTypeLimitLoader(&scene->compTypeLimits[i], i); // The limit may or may not be changed here.

            ZF4ComponentTypeInfo* compTypeInfo = zf4_get_component_type_info(i);

            scene->compArrays[i] = scene->memArena.push<ZF4Byte>(compTypeInfo->size * scene->compTypeLimits[i]);

            if (!scene->compArrays[i]) {
                return false;
            }

            scene->compActivities[i] = scene->memArena.push<ZF4Byte>(ZF4_BITS_TO_BYTES(scene->compTypeLimits[i]));

            if (!scene->compActivities[i]) {
                return false;
            }
        }
    }

    return true;
}

bool zf4_load_scene(ZF4Scene* scene, int sceneTypeIndex) {
    assert(zf4_is_zero(scene));

    zf4_log("Loading scene of type index %d...", sceneTypeIndex);

    scene->typeIndex = sceneTypeIndex;

    ZF4SceneTypeInfo* sceneTypeInfo = zf4_get_scene_type_info(sceneTypeIndex);

    if (!scene->memArena.init(sceneTypeInfo->memArenaSize)) {
        zf4_log_error("Failed to initialise scene main memory arena!");
        return false;
    }

    if (!scene->scratchSpace.init(ZF4_SCENE_SCRATCH_SPACE_SIZE)) {
        zf4_log_error("Failed to initialise scene scratch space memory arena!");
        return false;
    }

    if (!zf4_load_renderer(&scene->renderer, &scene->memArena, sceneTypeInfo->renderLayerCnt, sceneTypeInfo->camRenderLayerCnt, sceneTypeInfo->renderLayerPropsInitializer)) {
        zf4_log_error("Failed to load scene renderer!");
        return false;
    }

    if (!load_scene_ents(scene)) {
        zf4_log_error("Failed to load scene ents!");
        return false;
    }

    if (sceneTypeInfo->userDataSize > 0) {
        scene->userData = scene->memArena.push(sceneTypeInfo->userDataSize, sceneTypeInfo->userDataAlignment);

        if (!scene->userData) {
            zf4_log_error("Failed to reserve memory for scene user data!");
            return false;
        }
    }

    return sceneTypeInfo->init(scene);
}

void zf4_unload_scene(ZF4Scene* scene) {
    zf4_clean_renderer(&scene->renderer);
    scene->scratchSpace.clean();
    scene->memArena.clean();
    zf4_zero_out(scene);
}

bool zf4_proc_scene_tick(ZF4Scene* scene) {
    ZF4SceneTypeInfo* sceneTypeInfo = zf4_get_scene_type_info(scene->typeIndex);

    zf4_empty_sprite_batches(&scene->renderer);

    scene->scratchSpace.reset();

    int sceneChangeIndex = -1;

    if (!sceneTypeInfo->tick(scene, &sceneChangeIndex)) {
        return false;
    }
    
    if (sceneChangeIndex != -1) {
        zf4_log("Scene change request detected.");

        zf4_unload_scene(scene);

        if (!zf4_load_scene(scene, sceneChangeIndex)) {
            return false;
        }
    }

    return true;
}
