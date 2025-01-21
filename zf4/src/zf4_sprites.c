#include <zf4_sprites.h>

#if 0
namespace zf4 {
    bool LoadSprites(st_sprites& sprites, const int spriteCnt, const ta_get_sprite_load_info_func getLoadInfoFunc, struct mem_arena& memArena, struct mem_arena& scratchSpace, const s_textures& textures) {
        // NOTE: This is all probably too complicated. Maybe it should be simplified.
        assert(IsStructClear(sprites));

        //
        // Getting Sprite Information
        //
        struct array<SpriteLoadInfo> infos = {};

        if (!PushArrayToMemArena(infos, memArena, spriteCnt)) {
            return false;
        }

        int frameCntTotal = 0;

        for (int i = 0; i < spriteCnt; ++i) {
            infos[i] = getLoadInfoFunc(i);

            assert(infos[i].texIndex >= 0 && infos[i].texIndex < textures.cnt);
            assert(infos[i].frameCnt > 0);
            assert(infos[i].frameLoader);

            frameCntTotal += infos[i].frameCnt;
        }

        //
        // Setting Up Sprite Data
        //
        if (!PushArrayToMemArena<int>(sprites.tex_indexes, memArena, spriteCnt)) {
            return false;
        }

        if (!PushArrayToMemArena<st_rect_i>(sprites.frames, memArena, frameCntTotal)) {
            return false;
        }

        int frameIndex = 0;

        for (int i = 0; i < spriteCnt; ++i) {
            sprites.tex_indexes[i] = infos[i].texIndex;

            for (int j = 0; j < infos[i].frameCnt; ++j) {
                sprites.frames[frameCntTotal] = infos[i].frameLoader(j, textures);
            }

            frameIndex += infos[i].frameCnt;
        }

        return true;
    }

    struct rect_i ShrinkTexSrcRectToPixels(const s_rect_i& srcRect, const int texIndex, const s_textures& textures) {
        const s_vec_2d_i texSize = textures.sizes[texIndex]; // NOTE: Consider removing implicit conversion from Vec2DI to Vec2D.
        const s_array<unsigned char> texPxData = textures.px_datas[texIndex];

        // Determine the minimum source rectangle which encompasses all non-transparent pixels.
        int xMin = numeric_limits<int>::max();
        int xMax = numeric_limits<int>::min();
        
        int yMin = numeric_limits<int>::max();
        int yMax = numeric_limits<int>::min();

        for (int y = srcRect.y; y < srcRect.y + srcRect.height; ++y) {
            for (int x = srcRect.x; x < srcRect.x + srcRect.width; ++x) {
                const int pxDataIndex = ((y * texSize.x) + x) * g_tex_channel_cnt;
                const unsigned char pxAlpha = texPxData[pxDataIndex + 3];

                if (pxAlpha > 0) {
                    xMin = min(xMin, x);
                    xMax = max(xMax, x);

                    yMin = min(yMin, y);
                    yMax = max(yMax, y);
                }
            }
        }

        return {xMin, yMin, xMax - xMin + 1, yMax - yMin + 1};
    }
}
#endif
