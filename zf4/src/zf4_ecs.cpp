#include <zf4_ecs.h>

#include <zf4_game.h>
#include <zf4_sprites.h>

namespace zf4 {
    bool EntityManager::init(MemArena& memArena, const int entLimit, const Array<ComponentType>& compTypes) {
        assert(is_zero(this));
        assert(entLimit >= 0);

        if (entLimit > 0 && compTypes.get_len() > 0) {
            m_entLimit = entLimit;

            //
            // Entities
            //
            m_entPositions = memArena.alloc<Vec2D>(entLimit);

            if (!m_entPositions) {
                return false;
            }

            m_entCompIndexes = memArena.alloc<SafePtr<int>>(entLimit);

            if (!m_entCompIndexes) {
                return false;
            }

            for (int i = 0; i < entLimit; ++i) {
                m_entCompIndexes[i] = memArena.alloc<int>(compTypes.get_len());

                if (!m_entCompIndexes[i]) {
                    return false;
                }

                // WARNING: These are 0, though their default should really be -1.
            }

            m_entTags = memArena.alloc<int>(entLimit);

            if (!m_entTags) {
                return false;
            }

            m_entFlags = memArena.alloc<Byte>(entLimit);

            if (!m_entFlags) {
                return false;
            }

            m_entActivity = memArena.alloc<Byte>(bits_to_bytes(entLimit));

            if (!m_entActivity) {
                return false;
            }

            m_entVersions = memArena.alloc<int>(entLimit);

            if (!m_entVersions) {
                return false;
            }

            //
            // Components
            //
            m_compArrays = memArena.alloc<SafePtr<Byte>>(compTypes.get_len());

            if (!m_compArrays) {
                return false;
            }

            m_compActivities = memArena.alloc<SafePtr<Byte>>(compTypes.get_len());

            if (!m_compActivities) {
                return false;
            }

            for (int i = 0; i < compTypes.get_len(); ++i) {
                m_compArrays[i] = memArena.alloc<Byte>(compTypes.get(i).size * entLimit);

                if (!m_compArrays[i]) {
                    return false;
                }

                m_compActivities[i] = memArena.alloc<Byte>(bits_to_bytes(entLimit));

                if (!m_compActivities[i]) {
                    return false;
                }
            }
        }

        return true;
    }

    EntID EntityManager::spawn_ent(const Vec2D pos, const Array<ComponentType>& compTypes) {
        const int entIndex = get_first_inactive_bit_index(m_entActivity.get(), bits_to_bytes(m_entLimit));

        if (entIndex == -1) {
            return {};
        }

        activate_bit(m_entActivity.get(), entIndex);

        m_entPositions[entIndex] = pos;
        m_entTags[entIndex] = -1;

        for (int i = 0; i < compTypes.get_len(); ++i) {
            m_entCompIndexes[entIndex][i] = -1;
        }

        ++m_entVersions[entIndex];

        return {
            .index = entIndex,
            .version = m_entVersions[entIndex]
        };
    }

    EntID EntityManager::spawn_ent(const Vec2D pos, const SafePtr<Byte> compSig, const Array<ComponentType>& compTypes) {
        const EntID entID = spawn_ent(pos, compTypes);

        if (!entID.is_valid()) {
            return {};
        }

        if (!add_components_to_ent(compSig, compTypes, entID)) {
            destroy_ent(entID, compTypes);
            return {};
        }

        return entID;
    }

    void EntityManager::destroy_ent(const EntID entID, const Array<ComponentType>& compTypes) {
        assert(does_ent_exist(entID));

        deactivate_bit(m_entActivity.get(), entID.index);

        // Deactivate the entity's components.
        for (int i = 0; i < compTypes.get_len(); ++i) {
            const int compIndex = m_entCompIndexes[entID.index][i];

            if (compIndex != -1) {
                deactivate_bit(m_compActivities[i].get(), compIndex);
            }
        }
    }

    Byte* EntityManager::get_ent_component(const EntID entID, const int compTypeIndex, const Array<ComponentType>& compTypes) {
        assert(does_ent_exist(entID));
        assert(compTypeIndex >= 0 && compTypeIndex < compTypes.get_len());
        assert(does_ent_have_component(entID, compTypeIndex, compTypes));

        const int compIndex = m_entCompIndexes[entID.index][compTypeIndex];
        return m_compArrays[compTypeIndex].get() + (compIndex * compTypes.get(compTypeIndex).size);
    }

    bool EntityManager::add_component_to_ent(const int compTypeIndex, const Array<ComponentType>& compTypes, const EntID entID) {
        assert(does_ent_exist(entID));
        assert(compTypeIndex >= 0 && compTypeIndex < compTypes.get_len());
        assert(!does_ent_have_component(entID, compTypeIndex, compTypes));

        // Find and use the first inactive component of the type.
        const int compIndex = get_first_inactive_bit_index(m_compActivities[compTypeIndex].get(), bits_to_bytes(m_entLimit));

        if (compIndex == -1) {
            return false;
        }

        activate_bit(m_compActivities[compTypeIndex].get(), compIndex);

        m_entCompIndexes[entID.index][compTypeIndex] = compIndex;

        // Make sure the component data was zeroed out, and run the defaults loader function if defined.
        Byte* const comp = get_ent_component(entID, compTypeIndex, compTypes);
        assert(is_zero(comp, compTypes.get(compTypeIndex).size));

        if (compTypes.get(compTypeIndex).defaultsSetter) {
            compTypes.get(compTypeIndex).defaultsSetter(comp);
        }

        return true;
    }

    bool EntityManager::add_components_to_ent(const SafePtr<Byte> compSig, const Array<ComponentType>& compTypes, const EntID entID) {
        assert(does_ent_exist(entID));
        assert(!is_zero(compSig.get(), bits_to_bytes(compTypes.get_len())));

        for (int i = 0; i < compTypes.get_len(); ++i) {
            if (!is_bit_active(compSig.get(), i)) {
                continue;
            }

            if (!add_component_to_ent(i, compTypes, entID)) {
                return false;
            }
        }

        return true;
    }
}
