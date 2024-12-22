#include "zf4ap.h"

#include <cjson/cJSON.h>

typedef enum {
    TEXTURES_ASSET_TYPE,
    FONTS_ASSET_TYPE,
    SOUNDS_ASSET_TYPE,
    MUSIC_ASSET_TYPE
} AssetType;

cJSON* get_cj_asset_array(const cJSON* const instrsCJObj, const char* const arrayName) {
    cJSON* const assetsCJ = cJSON_GetObjectItemCaseSensitive(instrsCJObj, arrayName);

    if (!cJSON_IsArray(assetsCJ)) {
        return NULL;
    }

    return assetsCJ;
}

void write_asset_cnt(FILE* const outputFS, const cJSON* const cjAssetArray) {
    const int assetCnt = cJSON_GetArraySize(cjAssetArray);
    fwrite(&assetCnt, sizeof(assetCnt), 1, outputFS);
}

bool pack_textures(FILE* const outputFS, const cJSON* const instrsCJ, char* const srcAssetFilePathBuf, const int srcAssetFilePathStartLen, char* const errorMsgBuf) {
    const cJSON* const cjTextures = get_cj_assets_array(instrsCJ, "textures");

    if (!cjTextures) {
        return false;
    }

    write_asset_cnt(outputFS, cjTextures);

    const cJSON* cjTexRelFilePath = NULL;

    cJSON_ArrayForEach(cjTexRelFilePath, cjTextures) {
        // Get the relative file path of the texture.
        if (!cJSON_IsString(cjTexRelFilePath)) {
            return false;
        }

        if (!complete_asset_file_path(srcAssetFilePathBuf, srcAssetFilePathStartLen, cjTexRelFilePath->valuestring)) {
            return false;
        }

        // Load and write the size and pixel data of the texture.
        ZF4Pt2D texSize;
        stbi_uc* const texPxData = stbi_load(srcAssetFilePathBuf, &texSize.x, &texSize.y, nullptr, zf3::gk_texChannelCnt);

        if (!texPxData) {
            snprintf(errorMsgBuf, gk_errorMsgBufSize, "Failed to load pixel data for texture with relative file path \"%s\"!", cjTexRelFilePath->valuestring);
            return false;
        }

        fwrite(&texSize, sizeof(texSize), 1, outputFS);
        fwrite(texPxData, sizeof(*texPxData), texSize.x * texSize.y * zf3::gk_texChannelCnt, outputFS);

        zf4_log("Packed texture with file path \"%s\".", srcAssetFilePathBuf);

        stbi_image_free(texPxData);
    }

    return true;
}

int main(const int argCnt, const char* const* args) {
    if (argCnt != 3) {
        zf4_log_error("Invalid number of command-line arguments! Expected a source directory and an output directory.");
        return EXIT_FAILURE;
    }

    const char* const srcDir = args[1]; // The directory containing the assets to pack.
    const char* const outputDir = args[2]; // The directory to output the packed assets file to.

    return EXIT_SUCCESS;
}
