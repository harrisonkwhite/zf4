#include <zf4_ecs.h>

#include <zf4_game.h>
#include <zf4_sprites.h>

namespace zf4 {
    bool EntityManager::load(const int entLimit, const ComponentTypeLimitLoader compTypeLimitLoader, MemArena* const memArena) {
        assert(is_zero(this));
        assert(entLimit >= 0);

        if (entLimit > 0) {
            m_entLimit = entLimit;

            //
            // Entities
            //
            m_entPositions = memArena->push<Vec2D>(entLimit);

            if (!m_entPositions) {
                return false;
            }

            m_entColliderOffsets = memArena->push<RectEdges>(entLimit);

            if (!m_entColliderOffsets) {
                return false;
            }

            m_entCompIndexes = memArena->push<int*>(entLimit);

            if (!m_entCompIndexes) {
                return false;
            }

            for (int i = 0; i < entLimit; ++i) {
                m_entCompIndexes[i] = memArena->push<int>(get_component_type_cnt());

                if (!m_entCompIndexes[i]) {
                    return false;
                }

                // WARNING: These are zero!
            }

            m_entTags = memArena->push<int>(entLimit);

            if (!m_entTags) {
                return false;
            }

            m_entFlags = memArena->push<Byte>(entLimit);

            if (!m_entFlags) {
                return false;
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

                const ComponentTypeInfo* compTypeInfo = get_component_type_info(i);

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

        m_entPositions[entID->index] = pos;
        m_entTags[entID->index] = -1;

        for (int i = 0; i < get_component_type_cnt(); ++i) {
            m_entCompIndexes[entID->index][i] = -1;
        }

        ++m_entVersions[entID->index];
        entID->version = m_entVersions[entID->index];

        return true;
    }

    void EntityManager::destroy_ent(const EntID entID) {
        assert(does_ent_exist(entID));

        deactivate_bit(m_entActivity, entID.index);

        // Deactivate the entity's components.
        for (int i = 0; i < get_component_type_cnt(); ++i) {
            const int compIndex = m_entCompIndexes[entID.index][i];

            if (compIndex != -1) {
                deactivate_bit(m_compActivities[i], compIndex);
            }
        }
    }

    Rect EntityManager::create_ent_collider(const EntID entID, const int spriteIndex, const Vec2D origin, const Vec2D scale, const Vec2D posOffs) const {
        zf4::Rect collider;

        const zf4::Rect srcRect = get_game_sprites()[spriteIndex].frames[0]; // TEMP: Using just the first frame for now.
        collider.width = srcRect.width * scale.x;
        collider.height = srcRect.height * scale.y;

        const zf4::Vec2D entPos = get_ent_pos(entID);
        collider.x = entPos.x - (collider.width * origin.x);
        collider.y = entPos.y - (collider.height * origin.y);

        // Apply offsets.
        const zf4::RectEdges& colliderOffs = m_entColliderOffsets[entID.index];

        collider.x += colliderOffs.left + posOffs.x;
        collider.y += colliderOffs.top + posOffs.y;
        collider.width += colliderOffs.right - colliderOffs.left;
        collider.height += colliderOffs.bottom - colliderOffs.top;

        return collider;
    }

    Byte* EntityManager::get_ent_component(const EntID entID, const int compTypeIndex) {
        assert(does_ent_exist(entID));
        assert(compTypeIndex >= 0 && compTypeIndex < get_component_type_cnt());
        assert(does_ent_have_component(entID, compTypeIndex));

        const int compIndex = m_entCompIndexes[entID.index][compTypeIndex];
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

        m_entCompIndexes[entID.index][compTypeIndex] = compIndex;

        // Clear the component data, and run the defaults loader function if defined.
        const auto compTypeInfo = get_component_type_info(compTypeIndex);

        Byte* const comp = get_ent_component(entID, compTypeIndex);

        zero_out(comp, compTypeInfo->size);

        if (compTypeInfo->defaultsLoader) {
            compTypeInfo->defaultsLoader(comp);
        }

        return true;
    }
}
