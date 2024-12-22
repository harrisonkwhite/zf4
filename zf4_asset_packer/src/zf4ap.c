#include "zf4ap.h"

#include <cjson/cJSON.h>

#define PACKING_INSTRS_FILE_NAME "packing_instrs.json"

static const char* const i_assetTypeNames[] = {
    "textures",
    "fonts"
};

static const AssetTypePacker i_assetTypePackers[] = {
    pack_textures,
    pack_fonts
};

static FILE* open_output_fs(const char* const outputDir) {
    // Determine the output file path.
    char filePath[256];
    const int filePathLen = snprintf(filePath, sizeof(filePath), "%s/%s", outputDir, ZF4_ASSETS_FILE_NAME);

    if (filePathLen >= sizeof(filePath)) {
        zf4_log_error("Output file path \"%s\" is too long! Limit is %d characters.", filePath, sizeof(filePath) - 1);
        return NULL;
    }

    // Create or replace the output file.
    FILE* const fs = fopen(filePath, "wb");

    if (!fs) {
        zf4_log_error("Failed to open output file \"%s\"!", filePath);
    }

    return fs;
}

static char* get_packing_instrs_file_chars(char* const srcAssetFilePathBuf, const int srcAssetFilePathBufStartLen) {
    // Get the path of the packing instructions file.
    if (!complete_asset_file_path(srcAssetFilePathBuf, srcAssetFilePathBufStartLen, PACKING_INSTRS_FILE_NAME)) {
        return NULL;
    }

    // Get its contents (dynamically allocated).
    char* const contents = zf4_get_file_contents(srcAssetFilePathBuf);

    if (!contents) {
        zf4_log_error("Failed to get packing instructions file contents!", srcAssetFilePathBuf);
    }

    return contents;
}

bool run_asset_packer(AssetPacker* const packer, const char* const srcDir, const char* const outputDir) {
    assert(zf4_is_zero(packer, sizeof(*packer)));

    // Open the output file stream.
    packer->outputFS = open_output_fs(outputDir);

    if (!packer->outputFS) {
        return false;
    }

    // Initialise the source asset file path buffer with the source directory.
    char srcAssetFilePathBuf[SRC_ASSET_FILE_PATH_BUF_SIZE] = {0};
    const int srcAssetFilePathStartLen = snprintf(srcAssetFilePathBuf, SRC_ASSET_FILE_PATH_BUF_SIZE, "%s/", srcDir);

    if (srcAssetFilePathStartLen >= SRC_ASSET_FILE_PATH_BUF_SIZE) {
        return false;
    }

    // Get the contents of the packing instructions JSON file.
    packer->instrsFileChars = get_packing_instrs_file_chars(srcAssetFilePathBuf, srcAssetFilePathStartLen);

    if (!packer->instrsFileChars) {
        return false;
    }

    // Parse the packing instructions file contents.
    packer->instrsCJ = cJSON_Parse(packer->instrsFileChars);

    if (!packer->instrsCJ) {
        zf4_log_error("Failed to parse packing instructions JSON file \"%s\"!", srcAssetFilePathBuf);
        return false;
    }

    // Perform packing for each asset type using the packing instructions file.
    for (int i = 0; i < ZF4_ASSET_TYPE_CNT; ++i) {
        cJSON* const cjAssets = cJSON_GetObjectItemCaseSensitive(packer->instrsCJ, i_assetTypeNames[i]);

        if (!cJSON_IsArray(cjAssets)) {
            zf4_log_error("Failed to get \"%s\" array from packing instructions JSON file!", i_assetTypeNames[i]);
            return false;
        }

        const int assetCnt = cJSON_GetArraySize(cjAssets);
        fwrite(&assetCnt, sizeof(assetCnt), 1, packer->outputFS);

        if (assetCnt > 0) {
            if (!i_assetTypePackers[i](packer->outputFS, srcAssetFilePathBuf, srcAssetFilePathStartLen, cjAssets)) {
                return false;
            }
        }
    }

    return true;
}

void clean_asset_packer(AssetPacker* const packer, const bool packingSuccess) {
    cJSON_Delete(packer->instrsCJ);

    free(packer->instrsFileChars);

    if (packer->outputFS) {
        fclose(packer->outputFS);

        if (!packingSuccess) {
            remove(ZF4_ASSETS_FILE_NAME);
        }
    }

    memset(packer, 0, sizeof(*packer));
}

bool complete_asset_file_path(char* const srcAssetFilePathBuf, const int srcAssetFilePathStartLen, const char* const relPath) {
    strncpy(srcAssetFilePathBuf + srcAssetFilePathStartLen, relPath, SRC_ASSET_FILE_PATH_BUF_SIZE - srcAssetFilePathStartLen);

    if (srcAssetFilePathBuf[SRC_ASSET_FILE_PATH_BUF_SIZE - 1]) {
        const int lenLimit = SRC_ASSET_FILE_PATH_BUF_SIZE - 1 - srcAssetFilePathStartLen;
        zf4_log_error("Asset file path \"%s\" is too long! Limit is %d characters.", srcAssetFilePathBuf, lenLimit);
        return false;
    }

    return true;
}
