#include <zf4_scenes.h>

bool zf4_load_scene_of_type(ZF4SceneManager* const sceneManager, const int typeIndex) {
    assert(zf4_is_zero(&sceneManager->scene, sizeof(sceneManager->scene)));

    memset(&sceneManager->typeInfo, 0, sizeof(sceneManager->typeInfo));
    sceneManager->typeInfoLoader(&sceneManager->typeInfo, typeIndex);
    assert(!zf4_is_zero(&sceneManager->typeInfo, sizeof(sceneManager->typeInfo)));

    ZF4Scene* const scene = &sceneManager->scene;

    ZF4SceneTypeInfo typeInfo;
    sceneManager->typeInfoLoader(&typeInfo, typeIndex);

    if (!zf4_init_mem_arena(&scene->memArena, typeInfo.memArenaSize)) {
        zf4_log_error("Failed to initialise scene memory arena!");
        return false;
    }

    if (!zf4_load_renderer(&scene->renderer, &scene->memArena, typeInfo.renderLayerCnt, typeInfo.renderLayerPropsInitializer)) {
        zf4_log_error("Failed to load scene renderer!");
        zf4_clean_mem_arena(&scene->memArena); // NOTE: Unnecessary?
        return false;
    }

    return typeInfo.init(scene);
}

void zf4_unload_scene(ZF4Scene* const scene) {
    zf4_clean_renderer(&scene->renderer);
    zf4_clean_mem_arena(&scene->memArena);
    memset(scene, 0, sizeof(ZF4Scene));
}

bool zf4_proc_scene_tick(ZF4SceneManager* const sceneManager) {
    int sceneChangeIndex = -1;

    if (!sceneManager->typeInfo.tick(&sceneManager->scene, &sceneChangeIndex)) {
        return false;
    }

    if (sceneChangeIndex != -1) {
        zf4_unload_scene(&sceneManager->scene);

        if (!zf4_load_scene_of_type(sceneManager, sceneChangeIndex)) {
            return false;
        }
    }

    return true;
}
