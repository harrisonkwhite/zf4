#include <zf4_sprites.h>

void zf4_anim_tick(ZF4Anim* const anim) {
    const ZF4Sprite* const sprite = zf4_get_sprite(anim->spriteIndex);

    if (anim->frameTime < anim->frameInterval) {
        anim->frameTime++;
    } else {
        anim->frameIndex = (anim->frameIndex + 1) % sprite->frameCnt;
        anim->frameTime = 0;
    }
}
