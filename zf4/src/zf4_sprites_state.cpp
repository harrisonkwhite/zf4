#include <zf4_sprites.h>

#include <cstdlib>

namespace zf4 {
    static Sprite* i_sprites;
    static int i_spriteCnt;

    bool load_sprites(const int cnt, const SpriteLoader loader) {
        assert(!i_sprites);

        i_spriteCnt = cnt;

        if (cnt > 0) {
            i_sprites = alloc_zeroed<Sprite>(cnt);

            if (!i_sprites) {
                return false;
            }

            for (int i = 0; i < cnt; ++i) {
                loader(&i_sprites[i], i);
                assert(!is_zero(&i_sprites[i]));
            }
        }

        return true;
    }

    void unload_sprites() {
        if (i_sprites) {
            free(i_sprites);
            i_sprites = nullptr;
        }

        i_spriteCnt = 0;
    }

    const Sprite* get_sprite(const int index) {
        assert(i_sprites);
        assert(index >= 0 && index < i_spriteCnt);
        return &i_sprites[index];
    }
}
