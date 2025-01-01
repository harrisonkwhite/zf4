#include <zf4_sprites.h>

namespace zf4 {
    void anim_tick(Anim* const anim) {
        const Sprite* const sprite = get_sprite(anim->spriteIndex);

        if (anim->frameTime < anim->frameInterval) {
            anim->frameTime++;
        } else {
            anim->frameIndex = (anim->frameIndex + 1) % sprite->frameCnt;
            anim->frameTime = 0;
        }
    }
}
