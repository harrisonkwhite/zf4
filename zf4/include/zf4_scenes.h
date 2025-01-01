#pragma once

#include <zf4c.h>
#include <zf4_renderer.h>

namespace zf4 {
    constexpr int gk_sceneScratchSpaceSize = megabytes_to_bytes(2);

    using ComponentTypeLimitLoader = void (*)(int* const typeLimit, const int typeIndex);
    using ComponentDefaultsLoader = void (*)(Byte* const comp);

    struct ComponentTypeInfo {
        int size;
        int alignment;

        ComponentDefaultsLoader defaultsLoader;
    };

    using ComponentTypeInfoLoader = void (*)(ComponentTypeInfo* const typeInfo, const int typeIndex);

    struct EntID {
        int index;
        int version;
    };

    struct Scene;
    using OnEntDestroy = void (*)(const EntID entID, Scene* const scene);

    struct Ent {
        Vec2D pos;
        int* compIndexes;
        Byte* compSig; // NOTE: Remove?
        int tag;
        OnEntDestroy onDestroy;
    };

    class EntityManager {
    public:
        bool load(const int entLimit, const ComponentTypeLimitLoader compTypeLimitLoader, MemArena* const memArena);

        bool spawn_ent(EntID* const entID, const Vec2D pos);
        void destroy_ent(const EntID entID, Scene* const scene);
        int get_ents_with_tag(const int tag, EntID* const entIDs, const int entIDLimit) const;

        Byte* get_ent_component(const EntID entID, const int compTypeIndex);
        bool add_component_to_ent(const int compTypeIndex, const EntID entID);

        inline bool does_ent_exist(const EntID entID) const;
        inline Ent* get_ent(const EntID entID);
        inline const Ent* get_ent(const EntID entID) const;
        inline EntID create_ent_id(const int index) const;
        inline bool does_ent_have_component(const EntID entID, const int compTypeIndex);
        inline bool does_ent_have_tag(const EntID entID, const int tag);

    private:
        Ent* m_ents;
        Byte* m_entActivity;
        int* m_entVersions; // TODO: Move into the entity struct.
        int m_entLimit;

        Byte** m_compArrays; // One array per component type.
        int* m_compTypeLimits; // The maximum number of components of each type.
        Byte** m_compActivities; // One bitset per component type.
    };

    struct Scene {
        int typeIndex;

        MemArena memArena;
        MemArena scratchSpace;

        Renderer renderer;

        EntityManager entManager;

        void* userData;
    };

    using SceneInit = bool (*)(Scene* const scene);
    using SceneTick = bool (*)(Scene* const scene, int* const sceneChangeIndex);

    struct SceneTypeInfo {
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
    };

    using SceneTypeInfoLoader = void (*)(SceneTypeInfo* const typeInfo, const int typeIndex);

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

    inline bool EntityManager::does_ent_exist(const EntID entID) const {
        assert(entID.index >= 0 && entID.index < m_entLimit);
        return is_bit_active(m_entActivity, entID.index) && m_entVersions[entID.index] == entID.version;
    }

    inline Ent* EntityManager::get_ent(const EntID entID) {
        assert(does_ent_exist(entID));
        return &m_ents[entID.index];
    }

    inline const Ent* EntityManager::get_ent(const EntID entID) const {
        assert(does_ent_exist(entID));
        return &m_ents[entID.index];
    }

    inline EntID EntityManager::create_ent_id(const int index) const {
        assert(index >= 0 && index < m_entLimit);
        return {index, m_entVersions[index]};
    }

    inline bool EntityManager::does_ent_have_component(const EntID entID, const int compTypeIndex) {
        assert(does_ent_exist(entID));
        assert(compTypeIndex >= 0 && compTypeIndex < get_component_type_cnt());
        return get_ent(entID)->compIndexes[compTypeIndex] != -1;
    }

    inline bool EntityManager::does_ent_have_tag(const EntID entID, const int tag) {
        assert(does_ent_exist(entID));
        return get_ent(entID)->tag == tag;
    }
}
