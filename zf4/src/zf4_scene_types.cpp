#include <zf4_scenes.h>

#include <stdlib.h>

static int i_sceneTypeCnt;
static ZF4SceneTypeInfo* i_sceneTypeInfos;

bool zf4_load_scene_types(int typeCnt, ZF4SceneTypeInfoLoader typeInfoLoader) {
    if (typeCnt == 0) {
        return true;
    }

    assert(typeCnt > 0);

    i_sceneTypeInfos = zf4_alloc_zeroed<ZF4SceneTypeInfo>(typeCnt);

    if (!i_sceneTypeInfos) {
        return false;
    }

    i_sceneTypeCnt = typeCnt;

    for (int i = 0; i < typeCnt; i++) {
        typeInfoLoader(&i_sceneTypeInfos[i], i);
        assert(!zf4_is_zero(&i_sceneTypeInfos[i]));
    }

    return true;
}

void zf4_unload_scene_types() {
    i_sceneTypeCnt = 0;

    if (i_sceneTypeInfos) {
        free(i_sceneTypeInfos);
        i_sceneTypeInfos = nullptr;
    }
}

int zf4_get_scene_type_cnt() {
    return i_sceneTypeCnt;
}

ZF4SceneTypeInfo* zf4_get_scene_type_info(int typeIndex) {
    assert(typeIndex >= 0 && typeIndex < i_sceneTypeCnt);
    return &i_sceneTypeInfos[typeIndex];
}
