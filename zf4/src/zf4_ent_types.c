#include <zf4_scenes.h>

#include <stdlib.h>

static ZF4EntType* i_entTypes;
static int i_entTypeCnt;

bool zf4_load_ent_types(const int cnt, const ZF4EntTypeLoader loader) {
    assert(!i_entTypes);
    assert(cnt >= 0);

    if (cnt > 0) {
        i_entTypes = calloc(cnt, sizeof(*i_entTypes));
        i_entTypeCnt = cnt;

        if (!i_entTypes) {
            return false;
        }

        for (int i = 0; i < cnt; i++) {
            loader(&i_entTypes[i], i);
            assert(!zf4_is_zero(&i_entTypes[i], sizeof(i_entTypes[i])));
        }
    }

    return true;
}

void zf4_unload_ent_types() {
    if (i_entTypes) {
        free(i_entTypes);
        i_entTypes = NULL;
    }

    i_entTypeCnt = 0;
}

const ZF4EntType* zf4_get_ent_type(const int typeIndex) {
    assert(i_entTypes);
    assert(typeIndex >= 0 && typeIndex < i_entTypeCnt);
    return &i_entTypes[typeIndex];
}

int zf4_get_ent_type_cnt() {
    return i_entTypeCnt;
}
