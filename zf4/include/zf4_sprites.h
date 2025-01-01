#pragma once

#include <zf4c.h>

namespace zf4 {
    typedef void (*SpriteSrcRectLoader)(zf4::Rect* const srcRect, const int frameIndex);

    typedef struct {
        int texIndex;
        int frameCnt;
        SpriteSrcRectLoader srcRectLoader;
    } Sprite;

    typedef void (*SpriteLoader)(Sprite* const sprite, const int index);

    typedef struct {
        int spriteIndex;
        int frameIndex;
        int frameTime;
        int frameInterval;
    } Anim;

    //
    // State
    //
    bool load_sprites(const int cnt, const SpriteLoader loader);
    void unload_sprites();
    const Sprite* get_sprite(const int index);

    inline zf4::Rect get_sprite_src_rect(const int spriteIndex, const int frameIndex) {
        zf4::Rect rect;
        get_sprite(spriteIndex)->srcRectLoader(&rect, frameIndex);
        return rect;
    }

    inline zf4::Rect get_anim_src_rect(const Anim* const anim) {
        return get_sprite_src_rect(anim->spriteIndex, anim->frameIndex);
    }

    //
    // Logic
    //
    void anim_tick(Anim* const anim);
}
