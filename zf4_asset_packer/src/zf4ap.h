#pragma once

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cjson/cJSON.h>
#include <zf4c.h>

constexpr int gk_srcAssetFilePathBufSize = 256;

using AssetTypePacker = bool (*)(FILE* const outputFS, char* const srcAssetFilePathBuf, const int srcAssetFilePathStartLen, const cJSON* const cjAssets);

struct AssetPacker {
    FILE* outputFS;
    char* instrsFileChars;
    cJSON* instrsCJ;
};

bool run_asset_packer(AssetPacker* const packer, const char* const srcDir, const char* const outputDir);
void clean_asset_packer(AssetPacker* const packer, const bool packingSuccess);

bool complete_asset_file_path(char* const srcAssetFilePathBuf, const int srcAssetFilePathStartLen, const char* const relPath);

bool pack_textures(FILE* const outputFS, char* const srcAssetFilePathBuf, const int srcAssetFilePathStartLen, const cJSON* const cjTextures);
bool pack_fonts(FILE* const outputFS, char* const srcAssetFilePathBuf, const int srcAssetFilePathStartLen, const cJSON* const cjFonts);
bool pack_shaders(FILE* const outputFS, char* const srcAssetFilePathBuf, const int srcAssetFilePathStartLen, const cJSON* const cjProgs);
bool pack_sounds(FILE* const outputFS, char* const srcAssetFilePathBuf, const int srcAssetFilePathStartLen, const cJSON* const cjSnds);
bool pack_music(FILE* const outputFS, char* const srcAssetFilePathBuf, const int srcAssetFilePathStartLen, const cJSON* const cjMusic);
