#include <zf4_sprites.h>

#include <algorithm>
#include <zf4_assets.h>

namespace zf4 {
    RectI shrink_tex_src_rect_to_pixels(const RectI srcRect, const int texIndex) {
        const Vec2DI texSize = AssetManager::get_tex_size(texIndex); // NOTE: Consider removing implicit conversion from Vec2DI to Vec2D.
        const unsigned char* const texPxData = AssetManager::get_tex_px_data(texIndex);

        // Determine the minimum source rectangle which encompasses all non-transparent pixels.
        int xMin = INT_MAX;
        int xMax = INT_MIN;
        
        int yMin = INT_MAX;
        int yMax = INT_MIN;

        for (int y = srcRect.y; y < srcRect.y + srcRect.height; ++y) {
            for (int x = srcRect.x; x < srcRect.x + srcRect.width; ++x) {
                const int pxDataIndex = ((y * texSize.x) + x) * gk_texChannelCnt;
                const unsigned char pxAlpha = texPxData[pxDataIndex + 3];

                if (pxAlpha > 0) {
                    xMin = std::min(xMin, x);
                    xMax = std::max(xMax, x);

                    yMin = std::min(yMin, y);
                    yMax = std::max(yMax, y);
                }
            }
        }

        return {xMin, yMin, xMax - xMin + 1, yMax - yMin + 1};
    }

    void anim_tick(Anim* const anim, const Array<Sprite>* const sprites) {
        const Sprite* const sprite = &(*sprites)[anim->spriteIndex];

        if (anim->frameTime < anim->frameInterval) {
            anim->frameTime++;
        } else {
            anim->frameIndex = (anim->frameIndex + 1) % sprite->frames.get_len();
            anim->frameTime = 0;
        }
    }
}
