#pragma once

#include <cassert>
#include <zf4c.h>

namespace zf4 {
    using ComponentTypeLimitLoader = void (*)(int& typeLimit, const int typeIndex);

    struct ComponentType {
        int size;
        int alignment;

        void (*defaultsSetter)(Byte* const comp);
    };

    using ComponentTypeInitializer = void (*)(ComponentType& typeInfo, const int typeIndex);

    struct EntID {
        int index;
        int version;

        bool is_valid() const {
            return !zf4::is_zero(this);
        }
    };

    class EntityManager {
    public:
        bool init(MemArena& memArena, const int entLimit, const Array<ComponentType>& compTypes);

        EntID spawn_ent(const Vec2D pos, const Array<ComponentType>& compTypes);
        EntID spawn_ent(const Vec2D pos, const SafePtr<Byte> compSig, const Array<ComponentType>& compTypes);
        void destroy_ent(const EntID entID, const Array<ComponentType>& compTypes);
        Byte* get_ent_component(const EntID entID, const int compTypeIndex, const Array<ComponentType>& compTypes);
        bool add_component_to_ent(const int compTypeIndex, const Array<ComponentType>& compTypes, const EntID entID);
        bool add_components_to_ent(const SafePtr<Byte> compSig, const Array<ComponentType>& compTypes, const EntID entID);

        inline int get_ent_limit() const {
            return m_entLimit;
        }

        inline bool does_ent_exist(const EntID entID) const {
            assert(entID.index >= 0 && entID.index < m_entLimit);
            return is_bit_active(m_entActivity.get(), entID.index) && m_entVersions[entID.index] == entID.version;
        }

        inline EntID create_ent_id(const int index) const {
            assert(index >= 0 && index < m_entLimit);
            return {index, m_entVersions[index]};
        }

        inline Vec2D* get_ent_pos(const EntID entID) {
            assert(does_ent_exist(entID));
            return &m_entPositions[entID.index];
        }

        inline Vec2D get_ent_pos(const EntID entID) const {
            assert(does_ent_exist(entID));
            return m_entPositions[entID.index];
        }

        inline const Byte* get_ent_component(const EntID entID, const int compTypeIndex, const Array<ComponentType>& compTypes) const {
            assert(does_ent_exist(entID));
            return const_cast<EntityManager*>(this)->get_ent_component(entID, compTypeIndex, compTypes);
        }

        inline bool does_ent_have_component(const EntID entID, const int compTypeIndex, const Array<ComponentType>& compTypes) const {
            assert(does_ent_exist(entID));
            assert(compTypeIndex >= 0 && compTypeIndex < compTypes.get_len());
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
        SafePtr<Vec2D> m_entPositions;
        SafePtr<SafePtr<int>> m_entCompIndexes;
        SafePtr<int> m_entTags;
        SafePtr<Byte> m_entFlags;
        SafePtr<Byte> m_entActivity;
        SafePtr<int> m_entVersions;
        int m_entLimit;

        SafePtr<SafePtr<Byte>> m_compArrays; // One array per component type.
        SafePtr<SafePtr<Byte>> m_compActivities; // One bitset per component type.
    };
}
