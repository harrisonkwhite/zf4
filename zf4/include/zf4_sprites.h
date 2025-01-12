#pragma once

#include <zf4c.h>
#include <zf4_assets.h>

namespace zf4 {
    struct Sprite {
        int texIndex;
        Array<RectI> frames;
    };

    using SpritesLoader = bool (*)(Array<Sprite>* const sprites, MemArena* const memArena);

    RectI shrink_tex_src_rect_to_pixels(const RectI srcRect, const int texIndex, const Assets& assets);
}
