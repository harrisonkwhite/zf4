#include "zf4ap.h"

#include <cjson/cJSON.h>

static const char* const i_packing_instrs_file_name = "packing_instrs.json";

static const char* const i_asset_type_array_names[ecs_asset_type_cnt] = {
    "textures",
    "fonts",
    "shader_progs",
    "sounds",
    "music"
};

static const ta_asset_type_packer i_asset_type_packers[ecs_asset_type_cnt] = {
    PackTextures,
    PackFonts,
    PackShaders,
    PackSounds,
    PackMusic
};

static FILE* OpenOutputFS(const char* const output_dir) {
    // Determine the output file path.
    char file_path[256];
    const int file_path_len = snprintf(file_path, sizeof(file_path), "%s/%s", output_dir, ASSETS_FILE_NAME);

    if (file_path_len >= sizeof(file_path)) {
        LogError("Output file path \"%s\" is too long! Limit is %d characters.", file_path, sizeof(file_path) - 1);
        return NULL;
    }

    // Create or replace the output file.
    FILE* const fs = fopen(file_path, "wb");

    if (!fs) {
        LogError("Failed to open output file \"%s\"!", file_path);
    }

    return fs;
}

static char* GetPackingInstrsFileChars(char* const src_asset_file_path_buf, const int src_asset_file_path_buf_start_len) {
    // Get the path of the packing instructions file.
    if (!CompleteAssetFilePath(src_asset_file_path_buf, src_asset_file_path_buf_start_len, i_packing_instrs_file_name)) {
        return NULL;
    }

    // Get its contents (dynamically allocated).
    char* const contents = GetFileContents(src_asset_file_path_buf, NULL);

    if (!contents) {
        LogError("Failed to get packing instructions file contents!", src_asset_file_path_buf);
    }

    return contents;
}

bool RunAssetPacker(s_asset_packer* const packer, const char* const src_dir, const char* const output_dir) {
    assert(IsClear(packer, sizeof(*packer)));

    // Open the output file stream.
    packer->output_fs = OpenOutputFS(output_dir);

    if (!packer->output_fs) {
        return false;
    }

    // Initialise the source asset file path buffer with the source directory.
    char src_asset_file_path_buf[SRC_ASSET_FILE_PATH_BUF_SIZE] = {0};
    const int src_asset_file_path_start_len = snprintf(src_asset_file_path_buf, SRC_ASSET_FILE_PATH_BUF_SIZE, "%s/", src_dir);

    if (src_asset_file_path_start_len >= SRC_ASSET_FILE_PATH_BUF_SIZE) {
        return false;
    }

    // Get the contents of the packing instructions JSON file.
    packer->instrs_file_chars = GetPackingInstrsFileChars(src_asset_file_path_buf, src_asset_file_path_start_len);

    if (!packer->instrs_file_chars) {
        return false;
    }

    // Parse the packing instructions file contents.
    packer->instrs_cj = cJSON_Parse(packer->instrs_file_chars);

    if (!packer->instrs_cj) {
        LogError("Failed to parse packing instructions JSON file \"%s\"!", src_asset_file_path_buf);
        return false;
    }

    // Perform packing for each asset type using the packing instructions file.
    for (int i = 0; i < ecs_asset_type_cnt; ++i) {
        cJSON* const cj_assets = cJSON_GetObjectItemCaseSensitive(packer->instrs_cj, i_asset_type_array_names[i]);

        if (!cJSON_IsArray(cj_assets)) {
            LogError("Failed to get \"%s\" array from packing instructions JSON file!", i_asset_type_array_names[i]);
            return false;
        }

        const int asset_cnt = cJSON_GetArraySize(cj_assets);
        fwrite(&asset_cnt, sizeof(asset_cnt), 1, packer->output_fs);

        if (asset_cnt > 0) {
            if (!i_asset_type_packers[i](packer->output_fs, src_asset_file_path_buf, src_asset_file_path_start_len, cj_assets)) {
                return false;
            }
        }
    }

    return true;
}

void CleanAssetPacker(s_asset_packer* const packer, const bool packing_success) {
    cJSON_Delete(packer->instrs_cj);

    free(packer->instrs_file_chars);

    if (packer->output_fs) {
        fclose(packer->output_fs);

        if (!packing_success) {
            remove(ASSETS_FILE_NAME);
        }
    }

    Clear(packer, sizeof(*packer));
}

bool CompleteAssetFilePath(char* const src_asset_file_path_buf, const int src_asset_file_path_start_len, const char* const rel_path) {
    strncpy(src_asset_file_path_buf + src_asset_file_path_start_len, rel_path, SRC_ASSET_FILE_PATH_BUF_SIZE - src_asset_file_path_start_len);

    if (src_asset_file_path_buf[SRC_ASSET_FILE_PATH_BUF_SIZE - 1]) {
        const int len_limit = SRC_ASSET_FILE_PATH_BUF_SIZE - 1 - src_asset_file_path_start_len;
        LogError("Asset file path \"%s\" is too long! Limit is %d characters.", src_asset_file_path_buf, len_limit);
        return false;
    }

    return true;
}
