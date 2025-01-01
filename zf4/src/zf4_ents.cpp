#include <zf4_scenes.h>

namespace zf4 {
    bool EntityManager::load(const int entLimit, const ComponentTypeLimitLoader compTypeLimitLoader, MemArena* const memArena) {
        assert(is_zero(this));
        assert(entLimit >= 0);

        if (entLimit > 0) {
            m_entLimit = entLimit;

            //
            // Entities
            //
            m_ents = memArena->push<Ent>(entLimit);

            if (!m_ents) {
                return false;
            }

            for (int i = 0; i < entLimit; ++i) {
                m_ents[i].compIndexes = memArena->push<int>(get_component_type_cnt());

                if (!m_ents[i].compIndexes) {
                    return false;
                }

                m_ents[i].compSig = memArena->push<Byte>(bits_to_bytes(get_component_type_cnt()));

                if (!m_ents[i].compSig) {
                    return false;
                }
            }

            m_entActivity = memArena->push<Byte>(bits_to_bytes(entLimit));

            if (!m_entActivity) {
                return false;
            }

            m_entVersions = memArena->push<int>(entLimit);

            if (!m_entVersions) {
                return false;
            }

            //
            // Components
            //
            m_compArrays = memArena->push<Byte*>(get_component_type_cnt());

            if (!m_compArrays) {
                return false;
            }

            m_compTypeLimits = memArena->push<int>(get_component_type_cnt());

            if (!m_compTypeLimits) {
                return false;
            }

            m_compActivities = memArena->push<Byte*>(get_component_type_cnt());

            if (!m_compActivities) {
                return false;
            }

            for (int i = 0; i < get_component_type_cnt(); ++i) {
                m_compTypeLimits[i] = entLimit; // The default component type limit is the entity limit.
                compTypeLimitLoader(&m_compTypeLimits[i], i); // The limit may or may not be changed here.

                ComponentTypeInfo* compTypeInfo = get_component_type_info(i);

                m_compArrays[i] = memArena->push<Byte>(compTypeInfo->size * m_compTypeLimits[i]);

                if (!m_compArrays[i]) {
                    return false;
                }

                m_compActivities[i] = memArena->push<Byte>(bits_to_bytes(m_compTypeLimits[i]));

                if (!m_compActivities[i]) {
                    return false;
                }
            }
        }

        return true;
    }

    bool EntityManager::spawn_ent(EntID* const entID, const Vec2D pos) {
        assert(is_zero(entID));

        entID->index = get_first_inactive_bit_index(m_entActivity, m_entLimit);

        if (entID->index == -1) {
            return false;
        }

        activate_bit(m_entActivity, entID->index);

        Ent* ent = &m_ents[entID->index];

        ent->pos = pos;
        memset(ent->compIndexes, -1, sizeof(*ent->compIndexes) * get_component_type_cnt());
        memset(ent->compSig, 0, sizeof(*ent->compSig) * bits_to_bytes(get_component_type_cnt()));
        ent->tag = -1;
        ent->onDestroy = nullptr;

        ++m_entVersions[entID->index];
        entID->version = m_entVersions[entID->index];

        return true;
    }

    void EntityManager::destroy_ent(const EntID entID, Scene* const scene) {
        assert(does_ent_exist(entID));

        const Ent* const ent = &m_ents[entID.index];

        if (ent->onDestroy) {
            ent->onDestroy(entID, scene);
        }

        deactivate_bit(m_entActivity, entID.index);

        for (int i = 0; i < get_component_type_cnt(); ++i) {
            int compIndex = ent->compIndexes[i];

            if (compIndex != -1) {
                deactivate_bit(m_compActivities[i], compIndex);
            }
        }
    }

    int EntityManager::get_ents_with_tag(const int tag, EntID* const entIDs, const int entIDLimit) const {
        assert(entIDLimit > 0);

        int entCnt = 0;

        for (int i = 0; i < m_entLimit && entCnt < entIDLimit; ++i) {
            const EntID entID = {i, m_entVersions[i]};

            if (!does_ent_exist(entID)) {
                continue;
            }

            const Ent* const ent = &m_ents[entID.index];

            if (ent->tag == tag) {
                entIDs[entCnt] = entID;
                ++entCnt;
            }
        }

        return entCnt;
    }

    Byte* EntityManager::get_ent_component(const EntID entID, const int compTypeIndex) {
        assert(does_ent_exist(entID));
        assert(compTypeIndex >= 0 && compTypeIndex < get_component_type_cnt());
        assert(does_ent_have_component(entID, compTypeIndex));

        const int compIndex = m_ents[entID.index].compIndexes[compTypeIndex];
        const int compSize = get_component_type_info(compTypeIndex)->size;
        return m_compArrays[compTypeIndex] + (compIndex * compSize);
    }

    bool EntityManager::add_component_to_ent(const int compTypeIndex, const EntID entID) {
        assert(does_ent_exist(entID));
        assert(compTypeIndex >= 0 && compTypeIndex < get_component_type_cnt());
        assert(!does_ent_have_component(entID, compTypeIndex));

        // Find and use the first inactive component of the type.
        const int compIndex = get_first_inactive_bit_index(m_compActivities[compTypeIndex], m_compTypeLimits[compTypeIndex]);

        if (compIndex == -1) {
            return false;
        }

        activate_bit(m_compActivities[compTypeIndex], compIndex);

        // Update the component index of the entity.
        Ent* const ent = &m_ents[entID.index];
        ent->compIndexes[compTypeIndex] = compIndex;
        activate_bit(ent->compSig, compTypeIndex);

        // Clear the component data, and run the defaults loader function if defined.
        ComponentTypeInfo* const compTypeInfo = get_component_type_info(compTypeIndex);

        Byte* const comp = get_ent_component(entID, compTypeIndex);

        zero_out(comp, compTypeInfo->size);

        if (compTypeInfo->defaultsLoader) {
            compTypeInfo->defaultsLoader(comp);
        }

        return true;
    }
}
