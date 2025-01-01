#include <zf4_scenes.h>

#include <cstdlib>

namespace zf4 {
    static int i_sceneTypeCnt;
    static SceneTypeInfo* i_sceneTypeInfos;

    bool load_scene_types(int typeCnt, SceneTypeInfoLoader typeInfoLoader) {
        if (typeCnt == 0) {
            return true;
        }

        assert(typeCnt > 0);

        i_sceneTypeInfos = alloc_zeroed<SceneTypeInfo>(typeCnt);

        if (!i_sceneTypeInfos) {
            return false;
        }

        i_sceneTypeCnt = typeCnt;

        for (int i = 0; i < typeCnt; i++) {
            typeInfoLoader(&i_sceneTypeInfos[i], i);
            assert(!is_zero(&i_sceneTypeInfos[i]));
        }

        return true;
    }

    void unload_scene_types() {
        i_sceneTypeCnt = 0;

        if (i_sceneTypeInfos) {
            free(i_sceneTypeInfos);
            i_sceneTypeInfos = nullptr;
        }
    }

    int get_scene_type_cnt() {
        return i_sceneTypeCnt;
    }

    SceneTypeInfo* get_scene_type_info(int typeIndex) {
        assert(typeIndex >= 0 && typeIndex < i_sceneTypeCnt);
        return &i_sceneTypeInfos[typeIndex];
    }
}
