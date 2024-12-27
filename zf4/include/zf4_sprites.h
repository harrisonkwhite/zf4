#ifndef ZF4_ANIMS_H
#define ZF4_ANIMS_H

#include <zf4c.h>

typedef void (*ZF4SpriteSrcRectLoader)(ZF4Rect* const srcRect, const int frameIndex);

typedef struct {
    int texIndex;
    int frameCnt;
    ZF4SpriteSrcRectLoader srcRectLoader;
} ZF4Sprite;

typedef void (*ZF4SpriteLoader)(ZF4Sprite* const sprite, const int index);

typedef struct {
    int spriteIndex;
    int frameIndex;
    int frameTime;
    int frameInterval;
} ZF4Anim;

//
// State
//
bool zf4_load_sprites(const int cnt, const ZF4SpriteLoader loader);
void zf4_unload_sprites();
const ZF4Sprite* zf4_get_sprite(const int index);

inline ZF4Rect zf4_get_sprite_src_rect(const int spriteIndex, const int frameIndex) {
    ZF4Rect rect;
    zf4_get_sprite(spriteIndex)->srcRectLoader(&rect, frameIndex);
    return rect;
}

inline ZF4Rect zf4_get_anim_src_rect(const ZF4Anim* const anim) {
    return zf4_get_sprite_src_rect(anim->spriteIndex, anim->frameIndex);
}

//
// Logic
//
void zf4_anim_tick(ZF4Anim* const anim);

#endif
