#ifndef ZF4AP_H
#define ZF4AP_H

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <cjson/cJSON.h>
#include <zf4c.h>

#define SRC_ASSET_FILE_PATH_BUF_SIZE 256

cJSON* get_cj_asset_array(const cJSON* const instrsCJObj, const char* const arrayName);
void write_asset_cnt(FILE* const outputFS, const cJSON* const cjAssetArray);
bool complete_asset_file_path(char* const srcAssetFilePathBuf, const int srcAssetFilePathStartLen, const char* const relPath);

bool pack_textures(FILE* const outputFS, const cJSON* const instrsCJ, char* const srcAssetFilePathBuf, const int srcAssetFilePathStartLen);

#endif
