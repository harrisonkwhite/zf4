#include "zf4ap.h"

#include <cjson/cJSON.h>

static const char* const ik_packingInstrsFileName = "packing_instrs.json";

static const char* const ik_assetTypeArrayNames[] = {
    "textures",
    "fonts",
    "shader_progs",
    "sounds",
    "music"
};

static_assert(ZF4_STATIC_ARRAY_LEN(ik_assetTypeArrayNames) == zf4::ASSET_TYPE_CNT);

static const AssetTypePacker ik_assetTypePackers[zf4::ASSET_TYPE_CNT] = {
    pack_textures,
    pack_fonts,
    pack_shaders,
    pack_sounds,
    pack_music
};

static_assert(ZF4_STATIC_ARRAY_LEN(ik_assetTypePackers) == zf4::ASSET_TYPE_CNT);

static FILE* open_output_fs(const char* const outputDir) {
    // Determine the output file path.
    char filePath[256];
    const int filePathLen = snprintf(filePath, sizeof(filePath), "%s/%s", outputDir, zf4::gk_assetsFileName);

    if (filePathLen >= sizeof(filePath)) {
        zf4::log_error("Output file path \"%s\" is too long! Limit is %d characters.", filePath, sizeof(filePath) - 1);
        return nullptr;
    }

    // Create or replace the output file.
    FILE* const fs = fopen(filePath, "wb");

    if (!fs) {
        zf4::log_error("Failed to open output file \"%s\"!", filePath);
    }

    return fs;
}

static char* get_packing_instrs_file_chars(char* const srcAssetFilePathBuf, const int srcAssetFilePathBufStartLen) {
    // Get the path of the packing instructions file.
    if (!complete_asset_file_path(srcAssetFilePathBuf, srcAssetFilePathBufStartLen, ik_packingInstrsFileName)) {
        return nullptr;
    }

    // Get its contents (dynamically allocated).
    char* const contents = zf4::get_file_contents(srcAssetFilePathBuf);

    if (!contents) {
        zf4::log_error("Failed to get packing instructions file contents!", srcAssetFilePathBuf);
    }

    return contents;
}

bool run_asset_packer(AssetPacker* const packer, const char* const srcDir, const char* const outputDir) {
    assert(zf4::is_zero(packer));

    // Open the output file stream.
    packer->outputFS = open_output_fs(outputDir);

    if (!packer->outputFS) {
        return false;
    }

    // Initialise the source asset file path buffer with the source directory.
    char srcAssetFilePathBuf[gk_srcAssetFilePathBufSize] = {};
    const int srcAssetFilePathStartLen = snprintf(srcAssetFilePathBuf, gk_srcAssetFilePathBufSize, "%s/", srcDir);

    if (srcAssetFilePathStartLen >= gk_srcAssetFilePathBufSize) {
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
        zf4::log_error("Failed to parse packing instructions JSON file \"%s\"!", srcAssetFilePathBuf);
        return false;
    }

    // Perform packing for each asset type using the packing instructions file.
    for (int i = 0; i < zf4::ASSET_TYPE_CNT; ++i) {
        cJSON* const cjAssets = cJSON_GetObjectItemCaseSensitive(packer->instrsCJ, ik_assetTypeArrayNames[i]);

        if (!cJSON_IsArray(cjAssets)) {
            zf4::log_error("Failed to get \"%s\" array from packing instructions JSON file!", ik_assetTypeArrayNames[i]);
            return false;
        }

        const int assetCnt = cJSON_GetArraySize(cjAssets);
        fwrite(&assetCnt, sizeof(assetCnt), 1, packer->outputFS);

        if (assetCnt > 0) {
            if (!ik_assetTypePackers[i](packer->outputFS, srcAssetFilePathBuf, srcAssetFilePathStartLen, cjAssets)) {
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
            remove(zf4::gk_assetsFileName);
        }
    }

    memset(packer, 0, sizeof(*packer));
}

bool complete_asset_file_path(char* const srcAssetFilePathBuf, const int srcAssetFilePathStartLen, const char* const relPath) {
    strncpy(srcAssetFilePathBuf + srcAssetFilePathStartLen, relPath, gk_srcAssetFilePathBufSize - srcAssetFilePathStartLen);

    if (srcAssetFilePathBuf[gk_srcAssetFilePathBufSize - 1]) {
        const int lenLimit = gk_srcAssetFilePathBufSize - 1 - srcAssetFilePathStartLen;
        zf4::log_error("Asset file path \"%s\" is too long! Limit is %d characters.", srcAssetFilePathBuf, lenLimit);
        return false;
    }

    return true;
}
