#include <zf4_scenes.h>

#include <stdlib.h>

static int i_componentTypeCnt;
static ZF4ComponentTypeInfo* i_componentTypeInfos;

bool zf4_load_component_types(int typeCnt, ZF4ComponentTypeInfoLoader typeInfoLoader) {
    if (typeCnt == 0) {
        return true;
    }

    assert(typeCnt > 0);

    i_componentTypeInfos = calloc(typeCnt, sizeof(ZF4ComponentTypeInfo));

    if (!i_componentTypeInfos) {
        return false;
    }

    i_componentTypeCnt = typeCnt;

    for (int i = 0; i < typeCnt; i++) {
        typeInfoLoader(&i_componentTypeInfos[i], i);
        assert(!zf4_is_zero(&i_componentTypeInfos[i], sizeof(i_componentTypeInfos[i])));
    }

    return true;
}

void zf4_unload_component_types() {
    i_componentTypeCnt = 0;

    if (i_componentTypeInfos) {
        free(i_componentTypeInfos);
        i_componentTypeInfos = NULL;
    }
}

int zf4_get_component_type_cnt() {
    return i_componentTypeCnt;
}

ZF4ComponentTypeInfo* zf4_get_component_type_info(int typeIndex) {
    assert(typeIndex >= 0 && typeIndex < i_componentTypeCnt);
    return &i_componentTypeInfos[typeIndex];
}
