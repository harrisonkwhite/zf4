#include "zf4ap.h"

#include <cjson/cJSON.h>

#define PACKING_INSTRS_FILE_NAME "packing_instrs.json"

typedef struct {
    FILE* outputFS;
    char* instrsFileChars;
    cJSON* instrsCJ;
} AssetPacker;

static FILE* open_output_file(const char* const outputDir) {
    // Determine the output file path.
    char filePath[256];
    const int filePathLen = snprintf(filePath, sizeof(filePath), "%s/%s", outputDir, ZF4_ASSETS_FILE_NAME);

    if (filePathLen >= sizeof(filePath)) {
        return NULL;
    }

    // Create or replace the output file.
    return fopen(filePath, "wb");
}

static char* get_packing_instrs_file_chars(char* const srcAssetFilePathBuf, const int srcAssetFilePathBufStartLen) {
    if (!complete_asset_file_path(srcAssetFilePathBuf, srcAssetFilePathBufStartLen, PACKING_INSTRS_FILE_NAME)) {
        return NULL;
    }

    return zf4_get_file_contents(srcAssetFilePathBuf);
}

static bool run_asset_packer(AssetPacker* const packer, const char* const srcDir, const char* const outputDir) {
    assert(zf4_is_zero(packer, sizeof(*packer)));

    // Open the output file.
    packer->outputFS = open_output_file(outputDir);

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
        return false;
    }

    // Perform packing for each asset type using the packing instructions file.
    if (!pack_textures(packer->outputFS, packer->instrsCJ, srcAssetFilePathBuf, srcAssetFilePathStartLen)) {
        return false;
    }

    return true;
}

static void clean_asset_packer(AssetPacker* const packer, const bool packingSuccessful) {
    cJSON_Delete(packer->instrsCJ);

    free(packer->instrsFileChars);

    if (packer->outputFS) {
        fclose(packer->outputFS);

        if (!packingSuccessful) {
            remove(ZF4_ASSETS_FILE_NAME);
        }
    }
    
    memset(packer, 0, sizeof(*packer));
}

int main(const int argCnt, const char* const* args) {
    if (argCnt != 3) {
        zf4_log_error("Invalid number of command-line arguments! Expected a source directory and an output directory.");
        return EXIT_FAILURE;
    }

    const char* const srcDir = args[1]; // The directory containing the assets to pack.
    const char* const outputDir = args[2]; // The directory to output the packed assets file to.

    AssetPacker packer = {0};

    const bool packingSuccessful = run_asset_packer(&packer, srcDir, outputDir);

    clean_asset_packer(&packer, packingSuccessful);

    return packingSuccessful ? EXIT_SUCCESS : EXIT_FAILURE;
}

cJSON* get_cj_asset_array(const cJSON* const instrsCJObj, const char* const arrayName) {
    cJSON* const assets = cJSON_GetObjectItemCaseSensitive(instrsCJObj, arrayName);

    if (!cJSON_IsArray(assets)) {
        return NULL;
    }

    return assets;
}

void write_asset_cnt(FILE* const outputFS, const cJSON* const cjAssetArray) {
    const int assetCnt = cJSON_GetArraySize(cjAssetArray);
    fwrite(&assetCnt, sizeof(assetCnt), 1, outputFS);
}

bool complete_asset_file_path(char* const srcAssetFilePathBuf, const int srcAssetFilePathStartLen, const char* const relPath) {
    strncpy(srcAssetFilePathBuf + srcAssetFilePathStartLen, relPath, SRC_ASSET_FILE_PATH_BUF_SIZE - srcAssetFilePathStartLen);

    if (srcAssetFilePathBuf[SRC_ASSET_FILE_PATH_BUF_SIZE - 1]) {
        const int lenLimit = SRC_ASSET_FILE_PATH_BUF_SIZE - 1 - srcAssetFilePathStartLen;
        return false;
    }

    return true;
}
