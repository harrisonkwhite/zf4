#include <zf4_scenes.h>

#include <stdalign.h>
#include <zf4_sprites.h>

bool zf4_load_scene_of_type(ZF4SceneManager* const sceneManager, const int typeIndex, const ZF4GamePtrs* const gamePtrs) {
    assert(zf4_is_zero(&sceneManager->scene, sizeof(sceneManager->scene)));

    zf4_log("Loading scene of type index %d...", typeIndex);

    // Load scene type information.
    ZF4SceneTypeInfo* const sceneTypeInfo = &sceneManager->typeInfo;
    memset(sceneTypeInfo, 0, sizeof(*sceneTypeInfo));
    sceneManager->typeInfoLoader(sceneTypeInfo, typeIndex);
    assert(!zf4_is_zero(sceneTypeInfo, sizeof(*sceneTypeInfo)));

    // Set up various scene components.
    ZF4Scene* const scene = &sceneManager->scene;
    
    if (!zf4_init_mem_arena(&scene->memArena, sceneTypeInfo->memArenaSize)) {
        zf4_log_error("Failed to initialise scene memory arena!");
        return false;
    }

    if (!zf4_load_renderer(&scene->renderer, &scene->memArena, sceneTypeInfo->renderLayerCnt, sceneTypeInfo->camRenderLayerCnt, sceneTypeInfo->renderLayerPropsInitializer)) {
        zf4_log_error("Failed to load scene renderer!");
        return false;
    }

    if (!zf4_load_ents(&scene->entManager, &scene->memArena, sceneTypeInfo->entLimit, sceneTypeInfo->entTypeExtLimitLoader)) {
        zf4_log_error("Failed to load scene entities!");
        return false;
    }

    if (sceneTypeInfo->userDataSize > 0) {
        scene->userData = zf4_push_to_mem_arena(&scene->memArena, sceneTypeInfo->userDataSize, sceneTypeInfo->userDataAlignment);

        if (!scene->userData) {
            zf4_log_error("Failed to reserve memory for scene user data!");
            return false;
        }
    }

    return sceneTypeInfo->init(scene, gamePtrs);
}

void zf4_unload_scene(ZF4Scene* const scene) {
    zf4_clean_renderer(&scene->renderer);
    zf4_clean_mem_arena(&scene->memArena);
    memset(scene, 0, sizeof(ZF4Scene));
}

bool zf4_proc_scene_tick(ZF4SceneManager* const sceneManager, const ZF4GamePtrs* const gamePtrs) {
    ZF4Scene* const scene = &sceneManager->scene;

    zf4_empty_sprite_batches(&scene->renderer);

    // Call the tick function for each active entity.
    for (int i = 0; i < scene->entManager.entLimit; ++i) {
        if (!zf4_is_bit_active(scene->entManager.entActivityBitset, i)) {
            continue;
        }

        const ZF4EntType* const entType = zf4_get_ent_type(scene->entManager.ents[i].typeIndex);
        const ZF4EntID entID = {i, scene->entManager.entVersions[i]};

        if (!entType->tick(scene, entID, gamePtrs)) {
            return false;
        }
    }

    // Run the scene tick function.
    int sceneChangeIndex = -1;

    if (!sceneManager->typeInfo.tick(&sceneManager->scene, &sceneChangeIndex, gamePtrs)) {
        return false;
    }
    //

    if (sceneChangeIndex != -1) {
        zf4_log("Scene change request detected.");

        zf4_unload_scene(&sceneManager->scene);

        if (!zf4_load_scene_of_type(sceneManager, sceneChangeIndex, gamePtrs)) {
            return false;
        }
    }

    return true;
}
