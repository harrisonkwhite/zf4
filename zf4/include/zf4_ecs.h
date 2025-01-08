#pragma once

#include <cassert>
#include <zf4c.h>

namespace zf4 {
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

    bool load_component_types(const int typeCnt, const ComponentTypeInfoLoader typeInfoLoader);
    void unload_component_types();
    int get_component_type_cnt();
    const ComponentTypeInfo* get_component_type_info(const int typeIndex);

    // An entity is effectively an assortment of components, and a component is effectively a struct.
    class EntityManager {
    public:
        bool load(MemArena* const memArena, const int entLimit, const ComponentTypeLimitLoader compTypeLimitLoader);

        bool spawn_ent(EntID* const entID, const Vec2D pos);
        void destroy_ent(const EntID entID);
        Rect create_ent_collider(const EntID entID, const int spriteIndex, const Vec2D origin, const Vec2D scale, const Vec2D posOffs = {}) const;
        Byte* get_ent_component(const EntID entID, const int compTypeIndex);
        bool add_component_to_ent(const int compTypeIndex, const EntID entID);

        inline int get_ent_limit() const {
            return m_entLimit;
        }

        inline bool does_ent_exist(const EntID entID) const {
            assert(entID.index >= 0 && entID.index < m_entLimit);
            return is_bit_active(m_entActivity, entID.index) && m_entVersions[entID.index] == entID.version;
        }

        inline EntID create_ent_id(const int index) const {
            assert(index >= 0 && index < m_entLimit);
            return {index, m_entVersions[index]};
        }

        inline Vec2D& get_ent_pos(const EntID entID) {
            assert(does_ent_exist(entID));
            return m_entPositions[entID.index];
        }

        inline Vec2D get_ent_pos(const EntID entID) const {
            assert(does_ent_exist(entID));
            return m_entPositions[entID.index];
        }

        inline RectEdges& get_ent_collider_offset(const EntID entID) {
            assert(does_ent_exist(entID));
            return m_entColliderOffsets[entID.index];
        }

        inline RectEdges get_ent_collider_offset(const EntID entID) const {
            assert(does_ent_exist(entID));
            return m_entColliderOffsets[entID.index];
        }

        inline const Byte* get_ent_component(const EntID entID, const int compTypeIndex) const {
            assert(does_ent_exist(entID));
            return const_cast<EntityManager*>(this)->get_ent_component(entID, compTypeIndex);
        }

        inline bool does_ent_have_component(const EntID entID, const int compTypeIndex) const {
            assert(does_ent_exist(entID));
            assert(compTypeIndex >= 0 && compTypeIndex < get_component_type_cnt());
            return m_entCompIndexes[entID.index][compTypeIndex] != -1;
        }

        inline bool does_ent_have_tag(const EntID entID, const int tag) const {
            assert(does_ent_exist(entID));
            return m_entTags[entID.index] == tag;
        }

        inline void set_ent_tag(const EntID entID, const int tag) {
            assert(does_ent_exist(entID));
            assert(tag >= 0);
            m_entTags[entID.index] = tag;
        }

        inline Byte get_ent_flags(const EntID entID) const {
            assert(does_ent_exist(entID));
            return m_entFlags[entID.index];
        }

        inline void add_ent_flag(const EntID entID, const Byte flag) {
            assert(does_ent_exist(entID));
            assert(flag > 0);
            assert(is_power_of_two(flag));
            m_entFlags[entID.index] |= flag;
        }

    private:
        Vec2D* m_entPositions;
        RectEdges* m_entColliderOffsets;
        int** m_entCompIndexes;
        int* m_entTags;
        Byte* m_entFlags; // TEMP: For now we only allow 8 flags per entity.
        Byte* m_entActivity;
        int* m_entVersions;
        int m_entLimit;

        Byte** m_compArrays; // One array per component type.
        int* m_compTypeLimits; // The maximum number of components of each type.
        Byte** m_compActivities; // One bitset per component type.
    };
}
