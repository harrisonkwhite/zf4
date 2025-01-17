#pragma once

#include <zf4c.h>
#include <zf4_assets.h>

namespace zf4 {
    struct Sprite {
        int texIndex;
        Array<RectI> frames;
    };

    using SpriteInitializer = bool (*)(Sprite& sprite, MemArena& memArena, const int index, const zf4::Assets& assets);

    RectI shrink_tex_src_rect_to_pixels(const RectI& srcRect, const int texIndex, const Assets& assets);
}
