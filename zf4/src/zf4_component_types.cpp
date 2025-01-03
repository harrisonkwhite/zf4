#include <zf4_scenes.h>

#include <cstdlib>

namespace zf4 {
    static int i_componentTypeCnt;
    static ComponentTypeInfo* i_componentTypeInfos;

    bool load_component_types(const int typeCnt, const ComponentTypeInfoLoader typeInfoLoader) {
        if (typeCnt == 0) {
            return true;
        }

        assert(typeCnt > 0);

        i_componentTypeInfos = alloc_zeroed<ComponentTypeInfo>(typeCnt);

        if (!i_componentTypeInfos) {
            return false;
        }

        i_componentTypeCnt = typeCnt;

        for (int i = 0; i < typeCnt; i++) {
            typeInfoLoader(&i_componentTypeInfos[i], i);
            assert(!is_zero(&i_componentTypeInfos[i]));
        }

        return true;
    }

    void unload_component_types() {
        i_componentTypeCnt = 0;

        if (i_componentTypeInfos) {
            free(i_componentTypeInfos);
            i_componentTypeInfos = nullptr;
        }
    }

    int get_component_type_cnt() {
        return i_componentTypeCnt;
    }

    const ComponentTypeInfo* get_component_type_info(const int typeIndex) {
        assert(typeIndex >= 0 && typeIndex < i_componentTypeCnt);
        return &i_componentTypeInfos[typeIndex];
    }
}
