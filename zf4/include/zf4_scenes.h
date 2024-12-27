#ifndef ZF4_SCENES_H
#define ZF4_SCENES_H

#include <zf4c.h>
#include <zf4_renderer.h>
#include <zf4_audio.h>

typedef struct {
    ZF4Vec2D pos;

    int spriteIndex;
    ZF4Vec2D origin;

    int typeIndex;
    int typeExtIndex;
} ZF4Ent;

typedef struct {
    ZF4Ent* ents;
    int entLimit;
    ZF4Byte* entActivityBitset;
    int* entVersions;

    void** entTypeExtArrays;
    int* entTypeExtLimits;
    ZF4Byte** entTypeExtActivityBitsets;
} ZF4EntManager;

typedef struct {
    int index;
    int version;
} ZF4EntID;

typedef struct {
    ZF4MemArena memArena;
    ZF4Renderer renderer;
    ZF4EntManager entManager;
    void* userData;
} ZF4Scene;

typedef struct {
    ZF4SoundSrcManager* const sndSrcManager;
    ZF4MusicSrcManager* const musicSrcManager;
} ZF4GamePtrs;

typedef bool (*ZF4EntInit)(ZF4Scene* const scene, const ZF4EntID entID, const ZF4GamePtrs* const gamePtrs);
typedef bool (*ZF4EntTick)(ZF4Scene* const scene, const ZF4EntID entID, const ZF4GamePtrs* const gamePtrs);

typedef struct {
    int extSize;
    int extAlignment;

    ZF4EntInit init;
    ZF4EntTick tick;
} ZF4EntType;

typedef void (*ZF4EntTypeLoader)(ZF4EntType* const type, const int typeIndex);
typedef int (*ZF4EntTypeExtLimitLoader)(const int typeIndex);

typedef bool (*ZF4SceneInit)(ZF4Scene* const scene, const ZF4GamePtrs* const gamePtrs);
typedef bool (*ZF4SceneTick)(ZF4Scene* const scene, int* const sceneChangeIndex, const ZF4GamePtrs* const gamePtrs);

typedef struct {
    int memArenaSize;

    int renderLayerCnt;
    int camRenderLayerCnt;
    ZF4RenderLayerPropsInitializer renderLayerPropsInitializer;

    int entLimit;
    ZF4EntTypeExtLimitLoader entTypeExtLimitLoader;

    ZF4SceneInit init;
    ZF4SceneTick tick;

    int userDataSize;
    int userDataAlignment;
} ZF4SceneTypeInfo;

typedef void (*ZF4SceneTypeInfoLoader)(ZF4SceneTypeInfo* const typeInfo, const int typeIndex);

typedef struct {
    ZF4Scene scene;

    ZF4SceneTypeInfo typeInfo;
    ZF4SceneTypeInfoLoader typeInfoLoader;
} ZF4SceneManager;

bool zf4_load_ent_types(const int cnt, const ZF4EntTypeLoader loader);
void zf4_unload_ent_types();
const ZF4EntType* zf4_get_ent_type(const int typeIndex);
int zf4_get_ent_type_cnt();

bool zf4_load_ents(ZF4EntManager* const entManager, ZF4MemArena* const memArena, const int entLimit, const ZF4EntTypeExtLimitLoader entTypeExtLimitLoader);
ZF4EntID zf4_spawn_ent(ZF4Scene* const scene, const int typeIndex, const ZF4Vec2D pos, const ZF4GamePtrs* const gamePtrs);
void zf4_destroy_ent(ZF4EntManager* const entManager, const ZF4EntID id);
void zf4_write_ent_render_data(ZF4Renderer* const renderer, const ZF4Ent* const ent, const int layerIndex);
ZF4RectF zf4_get_ent_collider(const ZF4Ent* const ent);

bool zf4_load_scene_of_type(ZF4SceneManager* const sceneManager, const int typeIndex, const ZF4GamePtrs* const gamePtrs);
void zf4_unload_scene(ZF4Scene* const scene);
bool zf4_proc_scene_tick(ZF4SceneManager* const sceneManager, const ZF4GamePtrs* const gamePtrs);

inline bool zf4_does_ent_exist(const ZF4EntID id, const ZF4EntManager* const entManager) {
    return zf4_is_bit_active(entManager->entActivityBitset, id.index) && entManager->entVersions[id.index] == id.version;
}

inline ZF4Ent* zf4_get_ent(const ZF4EntID id, ZF4EntManager* const entManager) {
    assert(zf4_does_ent_exist(id, entManager));
    return &entManager->ents[id.index];
}

inline void* zf4_get_ent_type_ext(const ZF4EntID id, ZF4EntManager* const entManager) {
    assert(zf4_does_ent_exist(id, entManager));

    ZF4Ent* const ent = &entManager->ents[id.index];
    const ZF4EntType* const entType = zf4_get_ent_type(ent->typeIndex);
    return (ZF4Byte*)entManager->entTypeExtArrays[ent->typeIndex] + (ent->typeExtIndex * entType->extSize);
}

#endif
