#include <zf4_anims.h>

void zf4_anim_tick(ZF4Anim* const anim) {
    const ZF4AnimType* const type = zf4_get_anim_type(anim->typeIndex);

    if (anim->frameTime < type->frameInterval) {
        anim->frameTime++;
    } else {
        anim->frameIndex = (anim->frameIndex + 1) % type->frameCnt;
        anim->frameTime = 0;
    }
}

/*bool zf4_push_anim_types(ZF4AnimTypeManager* const manager, ZF4MemArena* const memArena, const ZF4AnimTypeLoader typeLoader, const int typeCnt) {
    manager->types = zf4_push_to_mem_arena(memArena, sizeof(*manager->types) * typeCnt, alignof(ZF4AnimType));

    if (!manager->types) {
        return false;
    }

    manager->typeCnt = typeCnt;

    for (int i = 0; i < typeCnt; ++i) {
        typeLoader(&manager->types[i], i);
        assert(!zf4_is_zero(&manager->types[i], sizeof(manager->types[i])));
    }

    return true;
}*/
