#ifndef ZF4_SCENES_H
#define ZF4_SCENES_H

#include <stdbool.h>
#include <zf4c.h>
#include <zf4_renderer.h>

#define ZF4_SCENE_SCRATCH_SPACE_SIZE ZF4_MEGABYTES(2)

typedef void (*ZF4ComponentTypeLimitLoader)(int* typeLimit, int typeIndex);
typedef void (*ZF4ComponentDefaultsLoader)(void* comp);

typedef struct {
    int size;
    int alignment;

    ZF4ComponentDefaultsLoader defaultsLoader;
} ZF4ComponentTypeInfo;

typedef void (*ZF4ComponentTypeInfoLoader)(ZF4ComponentTypeInfo* typeInfo, int typeIndex);

typedef struct ZF4EntID ZF4EntID;
typedef struct ZF4Scene ZF4Scene;
typedef void (*ZF4OnEntDestroy)(ZF4EntID entID, ZF4Scene* scene);

typedef struct {
    ZF4Vec2D pos;
    int* compIndexes;
    ZF4Byte* compSig;
    int tag;
    ZF4OnEntDestroy onDestroy;
} ZF4Ent;

typedef struct ZF4Scene {
    int typeIndex;

    ZF4MemArena memArena;
    ZF4MemArena scratchSpace;

    ZF4Renderer renderer;

    ZF4Ent* ents;
    ZF4Byte* entActivity;
    int* entVersions; // TODO: Move into the entity struct.

    void** compArrays; // One array per component type.
    ZF4Byte** compActivities; // One bitset per component type.
    int* compTypeLimits; // The maximum number of components of each type.

    void* userData;
} ZF4Scene;

typedef struct ZF4EntID {
    int index;
    int version;
} ZF4EntID;

typedef bool (*ZF4SceneInit)(ZF4Scene* const scene);
typedef bool (*ZF4SceneTick)(ZF4Scene* const scene, int* const sceneChangeIndex);

typedef struct {
    int memArenaSize;

    int renderLayerCnt;
    int camRenderLayerCnt;
    ZF4RenderLayerPropsInitializer renderLayerPropsInitializer;

    int entLimit;
    ZF4ComponentTypeLimitLoader compTypeLimitLoader;

    ZF4SceneInit init;
    ZF4SceneTick tick;

    int userDataSize;
    int userDataAlignment;
} ZF4SceneTypeInfo;

typedef void (*ZF4SceneTypeInfoLoader)(ZF4SceneTypeInfo* typeInfo, int typeIndex);

bool zf4_load_component_types(int typeCnt, ZF4ComponentTypeInfoLoader typeInfoLoader);
void zf4_unload_component_types();
int zf4_get_component_type_cnt();
ZF4ComponentTypeInfo* zf4_get_component_type_info(int typeIndex);

bool zf4_load_scene_types(int typeCnt, ZF4SceneTypeInfoLoader typeInfoLoader);
void zf4_unload_scene_types();
int zf4_get_scene_type_cnt();
ZF4SceneTypeInfo* zf4_get_scene_type_info(int typeIndex);

bool zf4_load_scene(ZF4Scene* scene, int typeIndex);
void zf4_unload_scene(ZF4Scene* scene);
bool zf4_proc_scene_tick(ZF4Scene* scene);

ZF4EntID zf4_spawn_ent(ZF4Vec2D pos, ZF4Scene* scene);
void zf4_destroy_ent(ZF4EntID entID, ZF4Scene* scene);
void* zf4_get_ent_component(ZF4EntID entID, int compTypeIndex, ZF4Scene* scene);
void* zf4_add_component_to_ent(int compTypeIndex, ZF4EntID entID, ZF4Scene* scene); // NOTE: Return value might be placeholder?
bool zf4_does_ent_have_component(ZF4EntID entID, int compTypeIndex, ZF4Scene* scene);
bool zf4_does_ent_have_component_signature(ZF4EntID entID, ZF4Byte* compSig, ZF4Scene* scene);
int zf4_get_ents_with_component_signature(ZF4EntID* entIDs, int entIDLimit, ZF4Byte* compSig, ZF4Scene* scene);
int zf4_get_ents_with_tag(ZF4EntID* entIDs, int entIDLimit, int tag, ZF4Scene* scene);

inline bool zf4_does_ent_exist(ZF4EntID entID, ZF4Scene* scene) {
    assert(entID.index >= 0 && entID.index < zf4_get_scene_type_info(scene->typeIndex)->entLimit);
    return zf4_is_bit_active(scene->entActivity, entID.index) && scene->entVersions[entID.index] == entID.version;
}

inline ZF4Ent* zf4_get_ent(ZF4EntID entID, ZF4Scene* scene) {
    assert(zf4_does_ent_exist(entID, scene));
    return &scene->ents[entID.index];
}

inline bool zf4_does_ent_have_component(ZF4EntID entID, int compTypeIndex, ZF4Scene* scene) {
    ZF4Ent* ent = zf4_get_ent(entID, scene);
    return ent->compIndexes[compTypeIndex] != -1;
}

inline ZF4Byte* zf4_push_component_signature(ZF4MemArena* memArena) {
    int compTypeCnt = zf4_get_component_type_cnt();
    return zf4_push_to_mem_arena(memArena, ZF4_BITS_TO_BYTES(compTypeCnt), 1);
}

#endif
