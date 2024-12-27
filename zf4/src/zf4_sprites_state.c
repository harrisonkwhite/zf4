#include <zf4_sprites.h>

#include <stdlib.h>

static ZF4Sprite* i_sprites;
static int i_spriteCnt;

bool zf4_load_sprites(const int cnt, const ZF4SpriteLoader loader) {
    assert(!i_sprites);

    i_spriteCnt = cnt;

    if (cnt > 0) {
        i_sprites = malloc(sizeof(*i_sprites) * cnt);

        if (!i_sprites) {
            return false;
        }

        for (int i = 0; i < cnt; ++i) {
            loader(&i_sprites[i], i);
            assert(!zf4_is_zero(&i_sprites[i], sizeof(i_sprites[i])));
        }
    }

    return true;
}

void zf4_unload_sprites() {
    if (i_sprites) {
        free(i_sprites);
        i_sprites = NULL;
    }

    i_spriteCnt = 0;
}

const ZF4Sprite* zf4_get_sprite(const int index) {
    assert(i_sprites);
    assert(index >= 0 && index < i_spriteCnt);
    return &i_sprites[index];
}
