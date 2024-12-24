#include <zf4_anims.h>

#include <stdlib.h>
#include <stdalign.h>

static ZF4AnimType* i_animTypes;
static int i_animTypeCnt;

bool zf4_load_anim_types(const int typeCnt, const ZF4AnimTypeLoader typeLoader) {
    assert(!i_animTypes);

    i_animTypeCnt = typeCnt;

    if (typeCnt > 0) {
        i_animTypes = malloc(sizeof(*i_animTypes) * typeCnt);

        if (!i_animTypes) {
            return false;
        }

        for (int i = 0; i < typeCnt; ++i) {
            typeLoader(&i_animTypes[i], i);
            assert(!zf4_is_zero(&i_animTypes[i], sizeof(i_animTypes[i])));
        }
    }

    return true;
}

void zf4_unload_anim_types() {
    if (i_animTypes) {
        free(i_animTypes);
        i_animTypes = NULL;
    }

    i_animTypeCnt = 0;
}

const ZF4AnimType* zf4_get_anim_type(const int index) {
    assert(i_animTypes);
    assert(index >= 0 && index < i_animTypeCnt);
    return &i_animTypes[index];
}
