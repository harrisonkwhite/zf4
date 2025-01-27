#pragma once

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cjson/cJSON.h>
#include <zf4c.h>

namespace zf4 {
    constexpr int g_src_asset_file_path_buf_size = 256;

    typedef bool (*ta_asset_type_packer)(FILE* const output_fs, char* const src_asset_file_path_buf, const int src_asset_file_path_start_len, const cJSON* const cj_assets);

    struct s_asset_packer {
        FILE* output_fs;
        char* instrs_file_chars;
        cJSON* instrs_cj;
    };

    bool RunAssetPacker(s_asset_packer& packer, const char* const src_dir, const char* const output_dir);
    void CleanAssetPacker(s_asset_packer& packer, const bool packing_success);

    bool CompleteAssetFilePath(char* const src_asset_file_path_buf, const int src_asset_file_path_start_len, const char* const rel_path);

    bool PackTextures(FILE* const output_fs, char* const src_asset_file_path_buf, const int src_asset_file_path_start_len, const cJSON* const cj_textures);
    bool PackFonts(FILE* const output_fs, char* const src_asset_file_path_buf, const int src_asset_file_path_start_len, const cJSON* const cj_fonts);
    bool PackShaders(FILE* const output_fs, char* const src_asset_file_path_buf, const int src_asset_file_path_start_len, const cJSON* const cj_progs);
    bool PackSounds(FILE* const output_fs, char* const src_asset_file_path_buf, const int src_asset_file_path_start_len, const cJSON* const cj_snds);
    bool PackMusic(FILE* const output_fs, char* const src_asset_file_path_buf, const int src_asset_file_path_start_len, const cJSON* const cj_music);
}
