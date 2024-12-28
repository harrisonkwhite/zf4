#include <zf4_scenes.h>

ZF4EntID zf4_spawn_ent(ZF4Vec2D pos, ZF4Scene* scene) {
    ZF4SceneTypeInfo* sceneTypeInfo = zf4_get_scene_type_info(scene->typeIndex);

    int entIndex = zf4_get_first_inactive_bit_index(scene->entActivity, sceneTypeInfo->entLimit);
    assert(entIndex != -1);

    zf4_activate_bit(scene->entActivity, entIndex);

    ZF4Ent* ent = &scene->ents[entIndex];

    ent->pos = pos;
    memset(ent->compIndexes, -1, sizeof(*ent->compIndexes) * zf4_get_component_type_cnt());
    memset(ent->compSig, 0, sizeof(*ent->compSig) * ZF4_BITS_TO_BYTES(zf4_get_component_type_cnt()));

    ++scene->entVersions[entIndex];

    return (ZF4EntID) { entIndex, scene->entVersions[entIndex] };
}

void zf4_destroy_ent(ZF4EntID entID, ZF4Scene* scene) {
    assert(false);
}

void* zf4_get_ent_component(ZF4EntID entID, int compTypeIndex, ZF4Scene* scene) {
    assert(zf4_does_ent_have_component(entID, compTypeIndex, scene));
    int compIndex = scene->ents[entID.index].compIndexes[compTypeIndex];
    int compSize = zf4_get_component_type_info(compTypeIndex)->size;
    return (ZF4Byte*)scene->compArrays[compTypeIndex] + (compIndex * compSize);
}

void* zf4_add_component_to_ent(int compTypeIndex, ZF4EntID entID, ZF4Scene* scene) {
    assert(!zf4_does_ent_have_component(entID, compTypeIndex, scene));

    ZF4SceneTypeInfo* sceneTypeInfo = zf4_get_scene_type_info(scene->typeIndex);

    int compIndex = zf4_get_first_inactive_bit_index(scene->compActivities[compTypeIndex], sceneTypeInfo->entLimit);
    assert(compIndex != -1);

    zf4_activate_bit(scene->compActivities[compTypeIndex], compIndex);

    ZF4Ent* ent = &scene->ents[entID.index];
    ent->compIndexes[compTypeIndex] = compIndex;
    zf4_activate_bit(ent->compSig, compTypeIndex);

    return zf4_get_ent_component(entID, compTypeIndex, scene);
}

bool zf4_does_ent_have_component_signature(ZF4EntID entID, ZF4Byte* compSig, ZF4Scene* scene) {
    ZF4Ent* ent = zf4_get_ent(entID, scene);

    for (int j = 0; j < ZF4_BITS_TO_BYTES(zf4_get_component_type_cnt()); ++j) {
        if ((ent->compSig[j] & compSig[j]) != compSig[j]) {
            return false;
        }
    }

    return true;
}

int zf4_get_ents_with_component_signature(ZF4EntID* entIDs, int entIDLimit, ZF4Byte* compSig, ZF4Scene* scene) {
    assert(entIDLimit > 0);

    int entCnt = 0;

    ZF4SceneTypeInfo* sceneTypeInfo = zf4_get_scene_type_info(scene->typeIndex);

    for (int i = 0; i < sceneTypeInfo->entLimit && entCnt < entIDLimit; ++i) {
        ZF4EntID entID = {i, scene->entVersions[i]};

        if (!zf4_does_ent_exist(entID, scene)) {
            continue;
        }

        if (zf4_does_ent_have_component_signature(entID, compSig, scene)) {
            entIDs[entCnt] = entID;
            ++entCnt;
        }
    }

    return entCnt;
}
