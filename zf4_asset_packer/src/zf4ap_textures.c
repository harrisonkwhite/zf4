#include "zf4ap.h"

#include <stb_image.h>

bool pack_textures(FILE* const outputFS, const cJSON* const instrsCJ, char* const srcAssetFilePathBuf, const int srcAssetFilePathStartLen) {
    const cJSON* const cjTextures = get_cj_asset_array(instrsCJ, "textures");

    if (!cjTextures) {
        return false;
    }

    write_asset_cnt(outputFS, cjTextures);

    const cJSON* cjTexRelFilePath = NULL;

    cJSON_ArrayForEach(cjTexRelFilePath, cjTextures) {
        if (!cJSON_IsString(cjTexRelFilePath)) {
            return false;
        }

        if (!complete_asset_file_path(srcAssetFilePathBuf, srcAssetFilePathStartLen, cjTexRelFilePath->valuestring)) {
            return false;
        }

        // Load and write the size and pixel data of the texture.
        ZF4Pt2D texSize;
        stbi_uc* const texPxData = stbi_load(srcAssetFilePathBuf, &texSize.x, &texSize.y, NULL, ZF4_TEX_CHANNEL_CNT);

        if (!texPxData) {
            return false;
        }

        fwrite(&texSize, sizeof(texSize), 1, outputFS);
        fwrite(texPxData, sizeof(*texPxData), texSize.x * texSize.y * ZF4_TEX_CHANNEL_CNT, outputFS);

        zf4_log("Packed texture with file path \"%s\".", srcAssetFilePathBuf);

        stbi_image_free(texPxData);
    }

    return true;
}
