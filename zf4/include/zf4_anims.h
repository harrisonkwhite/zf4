#ifndef ZF4_ANIMS_H
#define ZF4_ANIMS_H

#include <zf4c.h>

typedef void (*ZF4AnimSrcRectLoader)(ZF4Rect* const srcRect, const int frameIndex);

typedef struct {
    int texIndex;
    int frameCnt;
    int frameInterval;
    ZF4AnimSrcRectLoader srcRectLoader;
} ZF4AnimType;

typedef void (*ZF4AnimTypeLoader)(ZF4AnimType* const type, const int index);

/*typedef struct {
    ZF4AnimType* types;
    int typeCnt;
} ZF4AnimTypeManager;*/

typedef struct {
    int typeIndex;
    int frameIndex;
    int frameTime;
} ZF4Anim;

bool zf4_load_anim_types(const int typeCnt, const ZF4AnimTypeLoader typeLoader);
void zf4_unload_anim_types();
const ZF4AnimType* zf4_get_anim_type(const int index);

void zf4_anim_tick(ZF4Anim* const anim);

inline ZF4Rect zf4_get_anim_src_rect(const ZF4Anim* const anim) {
    ZF4Rect rect;
    zf4_get_anim_type(anim->typeIndex)->srcRectLoader(&rect, anim->frameIndex);
    return rect;
}

//bool zf4_push_anim_types(ZF4AnimTypeManager* const manager, ZF4MemArena* const memArena, const ZF4AnimTypeLoader typeLoader, const int typeCnt);

#endif
