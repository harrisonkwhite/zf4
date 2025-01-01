#include <zf4_scenes.h>

namespace zf4 {
    bool spawn_ent(EntID* const entID, const Vec2D pos, const Scene* const scene) {
        assert(is_zero(entID));

        const SceneTypeInfo* const sceneTypeInfo = get_scene_type_info(scene->typeIndex);

        entID->index = get_first_inactive_bit_index(scene->entActivity, sceneTypeInfo->entLimit);

        if (entID->index == -1) {
            return false;
        }

        activate_bit(scene->entActivity, entID->index);

        Ent* ent = &scene->ents[entID->index];

        ent->pos = pos;
        memset(ent->compIndexes, -1, sizeof(*ent->compIndexes) * get_component_type_cnt());
        memset(ent->compSig, 0, sizeof(*ent->compSig) * bits_to_bytes(get_component_type_cnt()));
        ent->tag = -1;
        ent->onDestroy = nullptr;

        ++scene->entVersions[entID->index];
        entID->version = scene->entVersions[entID->index];

        return true;
    }

    void destroy_ent(EntID entID, Scene* scene) {
        Ent* ent = get_ent(entID, scene);

        if (ent->onDestroy) {
            ent->onDestroy(entID, scene);
        }

        deactivate_bit(scene->entActivity, entID.index);

        for (int i = 0; i < get_component_type_cnt(); ++i) {
            int compIndex = ent->compIndexes[i];

            if (compIndex != -1) {
                deactivate_bit(scene->compActivities[i], compIndex);
            }
        }
    }

    void* get_ent_component(EntID entID, int compTypeIndex, Scene* scene) {
        assert(does_ent_have_component(entID, compTypeIndex, scene));
        int compIndex = scene->ents[entID.index].compIndexes[compTypeIndex];
        int compSize = get_component_type_info(compTypeIndex)->size;
        return (Byte*)scene->compArrays[compTypeIndex] + (compIndex * compSize);
    }

    bool add_component_to_ent(int compTypeIndex, EntID entID, Scene* scene) {
        assert(!does_ent_have_component(entID, compTypeIndex, scene));

        // Find and use the first inactive component of the type.
        SceneTypeInfo* sceneTypeInfo = get_scene_type_info(scene->typeIndex);
        int compIndex = get_first_inactive_bit_index(scene->compActivities[compTypeIndex], sceneTypeInfo->entLimit);

        if (compIndex == -1) {
            return false;
        }

        activate_bit(scene->compActivities[compTypeIndex], compIndex);

        // Update the component index of the entity.
        Ent* ent = &scene->ents[entID.index];
        ent->compIndexes[compTypeIndex] = compIndex;
        activate_bit(ent->compSig, compTypeIndex);

        // Clear the component data, and run the defaults loader function if defined.
        ComponentTypeInfo* compTypeInfo = get_component_type_info(compTypeIndex);
        void* comp = get_ent_component(entID, compTypeIndex, scene);
        memset(comp, 0, compTypeInfo->size);

        if (compTypeInfo->defaultsLoader) {
            compTypeInfo->defaultsLoader(comp);
        }

        return true;
    }

    bool does_ent_have_component_signature(EntID entID, Byte* compSig, Scene* scene) {
        Ent* ent = get_ent(entID, scene);

        for (int j = 0; j < bits_to_bytes(get_component_type_cnt()); ++j) {
            if ((ent->compSig[j] & compSig[j]) != compSig[j]) {
                return false;
            }
        }

        return true;
    }

    int get_ents_with_component_signature(EntID* entIDs, int entIDLimit, Byte* compSig, Scene* scene) {
        assert(entIDLimit > 0);

        int entCnt = 0;

        SceneTypeInfo* sceneTypeInfo = get_scene_type_info(scene->typeIndex);

        for (int i = 0; i < sceneTypeInfo->entLimit && entCnt < entIDLimit; ++i) {
            EntID entID = {i, scene->entVersions[i]};

            if (!does_ent_exist(entID, scene)) {
                continue;
            }

            if (does_ent_have_component_signature(entID, compSig, scene)) {
                entIDs[entCnt] = entID;
                ++entCnt;
            }
        }

        return entCnt;
    }

    int get_ents_with_tag(EntID* entIDs, int entIDLimit, int tag, Scene* scene) {
        assert(entIDLimit > 0);

        int entCnt = 0;

        SceneTypeInfo* sceneTypeInfo = get_scene_type_info(scene->typeIndex);

        for (int i = 0; i < sceneTypeInfo->entLimit && entCnt < entIDLimit; ++i) {
            EntID entID = {i, scene->entVersions[i]};

            if (!does_ent_exist(entID, scene)) {
                continue;
            }

            Ent* ent = get_ent(entID, scene);

            if (ent->tag == tag) {
                entIDs[entCnt] = entID;
                ++entCnt;
            }
        }

        return entCnt;
    }
}
