#ifndef ZF4_SCENES_H
#define ZF4_SCENES_H

#include <zf4c.h>
#include <zf4_assets.h>
#include <zf4_renderer.h>
#include <zf4_audio.h>
#include <zf4_anims.h>

typedef struct {
    ZF4MemArena memArena;
    ZF4Renderer renderer;
    void* miscPtr;
} ZF4Scene;

typedef struct {
    ZF4SoundSrcManager* const sndSrcManager;
    ZF4MusicSrcManager* const musicSrcManager;
} ZF4GamePtrs;

typedef bool (*ZF4SceneInit)(ZF4Scene* const scene, const ZF4GamePtrs* const gamePtrs);
typedef bool (*ZF4SceneTick)(ZF4Scene* const scene, int* const sceneChangeIndex, const ZF4GamePtrs* const gamePtrs);

typedef struct {
    int memArenaSize;

    int renderLayerCnt;
    int camRenderLayerCnt;
    ZF4RenderLayerPropsInitializer renderLayerPropsInitializer;

    ZF4SceneInit init;
    ZF4SceneTick tick;
} ZF4SceneTypeInfo;

typedef void (*ZF4SceneTypeInfoLoader)(ZF4SceneTypeInfo* const typeInfo, const int typeIndex);

typedef struct {
    ZF4Scene scene;

    ZF4SceneTypeInfo typeInfo;
    ZF4SceneTypeInfoLoader typeInfoLoader;
} ZF4SceneManager;

bool zf4_load_scene_of_type(ZF4SceneManager* const sceneManager, const int typeIndex, const ZF4GamePtrs* const gamePtrs);
void zf4_unload_scene(ZF4Scene* const scene);

bool zf4_proc_scene_tick(ZF4SceneManager* const sceneManager, const ZF4GamePtrs* const gamePtrs);

#endif
