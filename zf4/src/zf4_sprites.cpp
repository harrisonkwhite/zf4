#include <zf4_sprites.h>

#include <algorithm>

namespace zf4 {
    RectI shrink_tex_src_rect_to_pixels(const RectI srcRect, const int texIndex, const Assets& assets) {
        const Vec2DI texSize = assets.get_tex_size(texIndex); // NOTE: Consider removing implicit conversion from Vec2DI to Vec2D.
        const unsigned char* const texPxData = assets.get_tex_px_data(texIndex);

        // Determine the minimum source rectangle which encompasses all non-transparent pixels.
        int xMin = std::numeric_limits<int>::max();
        int xMax = std::numeric_limits<int>::min();
        
        int yMin = std::numeric_limits<int>::max();
        int yMax = std::numeric_limits<int>::min();

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
}
