#include "zf4ap.h"

#include <stb_image.h>

bool pack_textures(FILE* const outputFS, char* const srcAssetFilePathBuf, const int srcAssetFilePathStartLen, const cJSON* const cjTextures) {
    const cJSON* cjTexRelFilePath = nullptr;

    cJSON_ArrayForEach(cjTexRelFilePath, cjTextures) {
        if (!cJSON_IsString(cjTexRelFilePath)) {
            zf4_log_error("Failed to get texture file path from packing instructions JSON file!");
            return false;
        }

        if (!complete_asset_file_path(srcAssetFilePathBuf, srcAssetFilePathStartLen, cjTexRelFilePath->valuestring)) {
            return false;
        }

        // Load and write the size and pixel data of the texture.
        ZF4Pt2D texSize;
        stbi_uc* const texPxData = stbi_load(srcAssetFilePathBuf, &texSize.x, &texSize.y, nullptr, ZF4_TEX_CHANNEL_CNT);

        if (!texPxData) {
            zf4_log_error("Failed to load texture with file path \"%s\"!", srcAssetFilePathBuf);
            return false;
        }

        fwrite(&texSize, sizeof(texSize), 1, outputFS);
        fwrite(texPxData, sizeof(*texPxData), texSize.x * texSize.y * ZF4_TEX_CHANNEL_CNT, outputFS);

        zf4_log("Packed texture with file path \"%s\".", srcAssetFilePathBuf);

        stbi_image_free(texPxData);
    }

    return true;
}
