#include <zf4_scenes.h>

#include <stdalign.h>
#include <float.h>
#include <zf4c.h>
#include <zf4_sprites.h>

bool zf4_load_ents(ZF4EntManager* const entManager, ZF4MemArena* const memArena, const int entLimit, const ZF4EntTypeExtLimitLoader entTypeExtLimitLoader) {
    assert(zf4_is_zero(entManager, sizeof(*entManager)));

    if (entLimit == 0) {
        return true;
    }

    assert(entLimit > 0);

    entManager->entLimit = entLimit;

    // Reserve memory for base entity data.
    entManager->ents = zf4_push_to_mem_arena(memArena, sizeof(*entManager->ents) * entLimit, alignof(ZF4Ent));

    if (!entManager->ents) {
        return false;
    }

    // Reserve memory for entity activity bitset.
    entManager->entActivityBitset = zf4_push_to_mem_arena(memArena, ZF4_BITS_TO_BYTES(entLimit), 1);

    if (!entManager->entActivityBitset) {
        return false;
    }

    // Reserve memory for entity versions.
    entManager->entVersions = zf4_push_to_mem_arena(memArena, sizeof(*entManager->entVersions) * entLimit, alignof(int));

    if (!entManager->entVersions) {
        return false;
    }

    // Reserve memory for pointers to entity type extension arrays (one array per entity type).
    entManager->entTypeExtArrays = zf4_push_to_mem_arena(memArena, sizeof(*entManager->entTypeExtArrays) * zf4_get_ent_type_cnt(), alignof(void*));

    if (!entManager->entTypeExtArrays) {
        return false;
    }

    // Reserve memory for entity type extension limits.
    entManager->entTypeExtLimits = zf4_push_to_mem_arena(memArena, sizeof(*entManager->entTypeExtLimits) * zf4_get_ent_type_cnt(), alignof(int));

    if (!entManager->entTypeExtLimits) {
        return false;
    }

    // Reserve memory for pointers to entity type extension activity bitsets (one bitset per entity type).
    entManager->entTypeExtActivityBitsets = zf4_push_to_mem_arena(memArena, sizeof(*entManager->entTypeExtActivityBitsets) * zf4_get_ent_type_cnt(), alignof(ZF4Byte*));

    if (!entManager->entTypeExtActivityBitsets) {
        return false;
    }
    //

    for (int i = 0; i < zf4_get_ent_type_cnt(); ++i) {
        entManager->entTypeExtLimits[i] = entTypeExtLimitLoader(i);

        if (entManager->entTypeExtLimits[i] == 0) {
            continue;
        }

        // Reserve memory for the entity type extension array for this entity type.
        const ZF4EntType* const entType = zf4_get_ent_type(i);
        entManager->entTypeExtArrays[i] = zf4_push_to_mem_arena(memArena, entType->extSize * entManager->entTypeExtLimits[i], entType->extAlignment);

        if (!entManager->entTypeExtArrays[i]) {
            return false;
        }

        // Reserve memory for the entity type extension activity bitset.
        entManager->entTypeExtActivityBitsets[i] = zf4_push_to_mem_arena(memArena, ZF4_BITS_TO_BYTES(entManager->entTypeExtLimits[i]), 1);

        if (!entManager->entTypeExtActivityBitsets[i]) {
            return false;
        }
    }

    return true;
}

ZF4EntID zf4_spawn_ent(ZF4Scene* const scene, const int typeIndex, const ZF4Vec2D pos, const ZF4GamePtrs* const gamePtrs) {
    ZF4EntManager* const entManager = &scene->entManager;

    // Get the first inactive entity and use it.
    const int entIndex = zf4_get_first_inactive_bit_index(entManager->entActivityBitset, entManager->entLimit);
    assert(entIndex != -1);

    zf4_activate_bit(entManager->entActivityBitset, entIndex);

    // Get the first inactive entity type extension and use it.
    const int entTypeExtIndex = zf4_get_first_inactive_bit_index(entManager->entTypeExtActivityBitsets[typeIndex], entManager->entTypeExtLimits[typeIndex]);
    assert(entTypeExtIndex != -1);

    zf4_activate_bit(entManager->entTypeExtActivityBitsets[typeIndex], entTypeExtIndex);

    // Overwrite base entity data.
    entManager->ents[entIndex] = (ZF4Ent) {
        .pos = pos,
        .scale = {1.0f, 1.0f},
        .origin = {0.5f, 0.5f},
        .typeIndex = typeIndex,
        .typeExtIndex = entTypeExtIndex
    };
    //

    ++entManager->entVersions[entIndex];

    // Create an ID for the spawned entity.
    const ZF4EntID entID = (ZF4EntID) {entIndex, entManager->entVersions[entIndex]};

    // Call the initialisation function of the entity type.
    const ZF4EntType* const entType = zf4_get_ent_type(typeIndex);
    entType->init(scene, entID, gamePtrs);
    //

    return entID;
}

void zf4_destroy_ent(ZF4EntManager* const entManager, const ZF4EntID id) {
    zf4_deactivate_bit(entManager->entActivityBitset, id.index);

    ZF4Ent* const ent = &entManager->ents[id.index];
    zf4_deactivate_bit(entManager->entTypeExtActivityBitsets[ent->typeIndex], ent->typeExtIndex);
}

void zf4_write_ent_render_data(ZF4Renderer* const renderer, const ZF4Ent* const ent, const int layerIndex) {
    const ZF4SpriteBatchWriteInfo sbWriteInfo = {
        .texIndex = zf4_get_sprite(ent->spriteIndex)->texIndex,
        .pos = ent->pos,
        .srcRect = zf4_get_sprite_src_rect(ent->spriteIndex, 0),
        .origin = ent->origin,
        .rot = 0.0f,
        .scale = ent->scale,
        .alpha = 1.0f
    };

    zf4_write_to_sprite_batch(renderer, layerIndex, &sbWriteInfo);
}

ZF4RectF zf4_get_ent_collider(const ZF4Ent* const ent) {
    const ZF4Rect srcRect = zf4_get_sprite_src_rect(ent->spriteIndex, 0);
    const ZF4Pt2D srcRectSize = zf4_get_rect_size(&srcRect);
    const ZF4Vec2D size = {srcRectSize.x * ent->scale.x, srcRectSize.y * ent->scale.y};

    return (ZF4RectF) {
        ent->pos.x - (size.x * ent->origin.x),
        ent->pos.y - (size.y * ent->origin.y),
        size.x,
        size.y
    };
}

int zf4_get_colliding_ents(ZF4EntID* const collidingEntIDs, const int collidingEntIDLimit, const ZF4EntFilter entFilter, const ZF4EntID srcEntID, const ZF4EntManager* const entManager) {
    assert(collidingEntIDLimit > 0 && zf4_does_ent_exist(srcEntID, entManager));

    const ZF4Ent* const srcEnt = zf4_get_ent(entManager, srcEntID);
    const ZF4RectF srcEntCollider = zf4_get_ent_collider(srcEnt);

    int collidingEntIDCnt = 0;

    for (int i = 0; i < entManager->entLimit; ++i) {
        if (i == srcEntID.index) {
            continue;
        }
        
        if (!zf4_is_bit_active(entManager->entActivityBitset, i)) {
            continue;
        }

        const ZF4EntID entID = {i, entManager->entVersions[i]};

        if (entFilter && !entFilter(entID, entManager)) {
            continue;
        }

        const ZF4RectF collider = zf4_get_ent_collider(&entManager->ents[i]);

        if (zf4_do_rect_fs_intersect(&srcEntCollider, &collider)) {
            collidingEntIDs[collidingEntIDCnt] = entID;
            ++collidingEntIDCnt;

            if (collidingEntIDCnt == collidingEntIDLimit) {
                break;
            }
        }
    }

    return collidingEntIDCnt;
}

bool zf4_get_nearest_ent(ZF4EntID* const nearestEntID, const ZF4EntFilter entFilter, const ZF4EntID srcEntID, const ZF4EntManager* const entManager) {
    const ZF4Ent* const srcEnt = zf4_get_ent(entManager, srcEntID);
    
    float nearestDist;
    bool nearestDistSet = false;

    for (int i = 0; i < entManager->entLimit; ++i) {
        if (i == srcEntID.index) {
            continue;
        }

        if (!zf4_is_bit_active(entManager->entActivityBitset, i)) {
            continue;
        }

        const ZF4EntID entID = {i, entManager->entVersions[i]};

        if (entFilter && !entFilter(entID, entManager)) {
            continue;
        }

        const ZF4Ent* const ent = zf4_get_ent(entManager, entID);

        const float dist = zf4_calc_vec_2d_dist(srcEnt->pos, ent->pos);

        if (!nearestDistSet || dist < nearestDist) {
            *nearestEntID = entID;
            nearestDist = dist;
            nearestDistSet = true;
        }
    }

    return nearestDistSet;
}
