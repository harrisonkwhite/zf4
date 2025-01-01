#pragma once

#include <cstdbool>
#include <zf4c.h>
#include <zf4_renderer.h>

namespace zf4 {
    constexpr int gk_sceneScratchSpaceSize = megabytes_to_bytes(2);

    typedef void (*ComponentTypeLimitLoader)(int* typeLimit, int typeIndex);
    typedef void (*ComponentDefaultsLoader)(void* comp);

    typedef struct {
        int size;
        int alignment;

        ComponentDefaultsLoader defaultsLoader;
    } ComponentTypeInfo;

    typedef void (*ComponentTypeInfoLoader)(ComponentTypeInfo* typeInfo, int typeIndex);

    typedef struct EntID EntID;
    typedef struct Scene Scene;
    typedef void (*OnEntDestroy)(EntID entID, Scene* scene);

    typedef struct {
        Vec2D pos;
        int* compIndexes;
        Byte* compSig; // NOTE: Remove?
        int tag;
        OnEntDestroy onDestroy;
    } Ent;

    typedef struct Scene {
        int typeIndex;

        MemArena memArena;
        MemArena scratchSpace;

        Renderer renderer;

        Ent* ents;
        Byte* entActivity;
        int* entVersions; // TODO: Move into the entity struct.

        void** compArrays; // One array per component type.
        Byte** compActivities; // One bitset per component type.
        int* compTypeLimits; // The maximum number of components of each type.

        void* userData;
    } Scene;

    typedef struct EntID {
        int index;
        int version;
    } EntID;

    typedef bool (*SceneInit)(Scene* const scene);
    typedef bool (*SceneTick)(Scene* const scene, int* const sceneChangeIndex);

    typedef struct {
        int memArenaSize;

        int renderLayerCnt;
        int camRenderLayerCnt;
        RenderLayerPropsInitializer renderLayerPropsInitializer;

        int entLimit;
        ComponentTypeLimitLoader compTypeLimitLoader;

        SceneInit init;
        SceneTick tick;

        int userDataSize;
        int userDataAlignment;
    } SceneTypeInfo;

    typedef void (*SceneTypeInfoLoader)(SceneTypeInfo* typeInfo, int typeIndex);

    bool load_component_types(int typeCnt, ComponentTypeInfoLoader typeInfoLoader);
    void unload_component_types();
    int get_component_type_cnt();
    ComponentTypeInfo* get_component_type_info(int typeIndex);

    bool load_scene_types(int typeCnt, SceneTypeInfoLoader typeInfoLoader);
    void unload_scene_types();
    int get_scene_type_cnt();
    SceneTypeInfo* get_scene_type_info(int typeIndex);

    bool load_scene(Scene* scene, int typeIndex);
    void unload_scene(Scene* scene);
    bool proc_scene_tick(Scene* scene);

    bool spawn_ent(EntID* const entID, const Vec2D pos, const Scene* const scene);
    void destroy_ent(EntID entID, Scene* scene);
    void* get_ent_component(EntID entID, int compTypeIndex, Scene* scene);
    bool add_component_to_ent(int compTypeIndex, EntID entID, Scene* scene);
    bool does_ent_have_component(EntID entID, int compTypeIndex, Scene* scene);
    bool does_ent_have_component_signature(EntID entID, Byte* compSig, Scene* scene);
    int get_ents_with_component_signature(EntID* entIDs, int entIDLimit, Byte* compSig, Scene* scene);
    int get_ents_with_tag(EntID* entIDs, int entIDLimit, int tag, Scene* scene);

    inline EntID create_ent_id(int entIndex, Scene* scene) {
        return {
            .index = entIndex,
            .version = scene->entVersions[entIndex]
        };
    }

    inline bool does_ent_exist(EntID entID, Scene* scene) {
        assert(entID.index >= 0 && entID.index < get_scene_type_info(scene->typeIndex)->entLimit);
        return is_bit_active(scene->entActivity, entID.index) && scene->entVersions[entID.index] == entID.version;
    }

    inline Ent* get_ent(EntID entID, Scene* scene) {
        assert(does_ent_exist(entID, scene));
        return &scene->ents[entID.index];
    }

    inline bool does_ent_have_component(EntID entID, int compTypeIndex, Scene* scene) {
        Ent* ent = get_ent(entID, scene);
        return ent->compIndexes[compTypeIndex] != -1;
    }

    inline bool does_ent_have_tag(EntID entID, int tag, Scene* scene) {
        Ent* ent = get_ent(entID, scene);
        return ent->tag == tag;
    }

    inline Byte* push_component_signature(MemArena* memArena) {
        int compTypeCnt = get_component_type_cnt();
        return memArena->push<Byte>(bits_to_bytes(compTypeCnt));
    }
}
