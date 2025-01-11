#pragma once

#include <zf4c.h>
#include <zf4_assets.h>

namespace zf4 {
    struct Sprite {
        int texIndex;
        Array<RectI> frames;
    };

    using SpritesLoader = bool (*)(Array<Sprite>* const sprites, MemArena* const memArena);

    struct Anim {
        int spriteIndex;
        int frameIndex;
        int frameTime;
        int frameInterval;
    };

    RectI shrink_tex_src_rect_to_pixels(const RectI srcRect, const int texIndex, const Assets& assets);
    void anim_tick(Anim* const anim, const Array<Sprite>* const sprites);
}
