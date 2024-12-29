#include <zf4_scenes.h>

#include <stdalign.h>

static bool load_scene_ent(ZF4Scene* scene, ZF4Ent* ent) {
    ent->compIndexes = zf4_push_to_mem_arena(&scene->memArena, sizeof(*ent->compIndexes) * zf4_get_component_type_cnt(), alignof(int));

    if (!ent->compIndexes) {
        return false;
    }

    ent->compSig = zf4_push_to_mem_arena(&scene->memArena, ZF4_BITS_TO_BYTES(zf4_get_component_type_cnt()), alignof(ZF4Byte));

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
        scene->ents = zf4_push_to_mem_arena(&scene->memArena, sizeof(*scene->ents) * sceneTypeInfo->entLimit, alignof(ZF4Ent));

        if (!scene->ents) {
            return false;
        }

        for (int i = 0; i < sceneTypeInfo->entLimit; ++i) {
            if (!load_scene_ent(scene, &scene->ents[i])) {
                return false;
            }
        }

        scene->entActivity = zf4_push_to_mem_arena(&scene->memArena, ZF4_BITS_TO_BYTES(sceneTypeInfo->entLimit), alignof(ZF4Byte));

        if (!scene->entActivity) {
            return false;
        }

        scene->entVersions = zf4_push_to_mem_arena(&scene->memArena, sizeof(*scene->entVersions) * sceneTypeInfo->entLimit, alignof(int));

        if (!scene->entVersions) {
            return false;
        }

        //
        // Components
        //
        scene->compArrays = zf4_push_to_mem_arena(&scene->memArena, sizeof(*scene->compArrays) * zf4_get_component_type_cnt(), alignof(void*));

        if (!scene->compArrays) {
            return false;
        }

        scene->compActivities = zf4_push_to_mem_arena(&scene->memArena, sizeof(*scene->compActivities) * zf4_get_component_type_cnt(), alignof(ZF4Byte*));

        if (!scene->compActivities) {
            return false;
        }

        scene->compTypeLimits = zf4_push_to_mem_arena(&scene->memArena, sizeof(*scene->compTypeLimits) * zf4_get_component_type_cnt(), alignof(int));

        if (!scene->compTypeLimits) {
            return false;
        }

        for (int i = 0; i < zf4_get_component_type_cnt(); ++i) {
            scene->compTypeLimits[i] = sceneTypeInfo->entLimit; // The default component type limit is the entity limit.
            sceneTypeInfo->compTypeLimitLoader(&scene->compTypeLimits[i], i);

            ZF4ComponentTypeInfo* compTypeInfo = zf4_get_component_type_info(i);

            scene->compArrays[i] = zf4_push_to_mem_arena(&scene->memArena, compTypeInfo->size * scene->compTypeLimits[i], compTypeInfo->alignment);

            if (!scene->compArrays[i]) {
                return false;
            }

            scene->compActivities[i] = zf4_push_to_mem_arena(&scene->memArena, ZF4_BITS_TO_BYTES(scene->compTypeLimits[i]), alignof(ZF4Byte));

            if (!scene->compActivities[i]) {
                return false;
            }
        }
    }

    return true;
}

bool zf4_load_scene(ZF4Scene* scene, int sceneTypeIndex) {
    assert(zf4_is_zero(scene, sizeof(*scene)));

    zf4_log("Loading scene of type index %d...", sceneTypeIndex);

    scene->typeIndex = sceneTypeIndex;

    ZF4SceneTypeInfo* sceneTypeInfo = zf4_get_scene_type_info(sceneTypeIndex);

    if (!zf4_init_mem_arena(&scene->memArena, sceneTypeInfo->memArenaSize)) {
        zf4_log_error("Failed to initialise scene main memory arena!");
        return false;
    }

    if (!zf4_init_mem_arena(&scene->scratchSpace, ZF4_SCENE_SCRATCH_SPACE_SIZE)) {
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
        scene->userData = zf4_push_to_mem_arena(&scene->memArena, sceneTypeInfo->userDataSize, sceneTypeInfo->userDataAlignment);

        if (!scene->userData) {
            zf4_log_error("Failed to reserve memory for scene user data!");
            return false;
        }
    }

    return sceneTypeInfo->init(scene);
}

void zf4_unload_scene(ZF4Scene* scene) {
    zf4_clean_renderer(&scene->renderer);
    zf4_clean_mem_arena(&scene->scratchSpace);
    zf4_clean_mem_arena(&scene->memArena);
    memset(scene, 0, sizeof(*scene));
}

bool zf4_proc_scene_tick(ZF4Scene* scene) {
    ZF4SceneTypeInfo* sceneTypeInfo = zf4_get_scene_type_info(scene->typeIndex);

    zf4_empty_sprite_batches(&scene->renderer);

    zf4_reset_mem_arena(&scene->scratchSpace);

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
