#pragma once

#include <zf4c.h>

namespace zf4 {
    using SpriteSrcRectLoader = void (*)(zf4::RectI* const srcRect, const int frameIndex);

    struct Sprite {
        int texIndex;
        int frameCnt;
        SpriteSrcRectLoader srcRectLoader;
    };

    using SpriteLoader = void (*)(Sprite* const sprite, const int index);

    struct Anim {
        int spriteIndex;
        int frameIndex;
        int frameTime;
        int frameInterval;
    };

    //
    // State
    //
    bool load_sprites(const int cnt, const SpriteLoader loader);
    void unload_sprites();
    const Sprite* get_sprite(const int index);

    inline zf4::RectI get_sprite_src_rect(const int spriteIndex, const int frameIndex) {
        zf4::RectI rect;
        get_sprite(spriteIndex)->srcRectLoader(&rect, frameIndex);
        return rect;
    }

    inline zf4::RectI get_anim_src_rect(const Anim* const anim) {
        return get_sprite_src_rect(anim->spriteIndex, anim->frameIndex);
    }

    //
    // Logic
    //
    void anim_tick(Anim* const anim);
}
