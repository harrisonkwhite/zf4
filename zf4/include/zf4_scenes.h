#ifndef ZF4_SCENES_H
#define ZF4_SCENES_H

#include <zf4c.h>
#include <zf4_renderer.h>

typedef struct {
    ZF4MemArena memArena;
    ZF4Renderer renderer;
} ZF4Scene;

typedef bool (*ZF4SceneTick)(ZF4Scene* const scene, int* const sceneChangeIndex);

typedef struct {
    int memArenaSize;

    int renderLayerCnt;
    ZF4RenderLayerPropsInitializer renderLayerPropsInitializer;

    ZF4SceneTick tick;
} ZF4SceneTypeInfo;

typedef void (*ZF4SceneTypeInfoLoader)(ZF4SceneTypeInfo* const typeInfo, const int typeIndex);

typedef struct {
    ZF4Scene scene;

    ZF4SceneTypeInfo typeInfo;
    ZF4SceneTypeInfoLoader typeInfoLoader;
} ZF4SceneManager;

bool zf4_load_scene_of_type(ZF4SceneManager* const sceneManager, const int typeIndex);
void zf4_unload_scene(ZF4Scene* const scene);

bool zf4_proc_scene_tick(ZF4SceneManager* const sceneManager);

#endif
