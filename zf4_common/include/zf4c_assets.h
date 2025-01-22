#ifndef ZF4C_ASSETS_H
#define ZF4C_ASSETS_H

#include <zf4c_math.h>

#define ASSETS_FILE_NAME "assets.dat"

#define TEXTURE_WIDTH_LIMIT 2048
#define TEXTURE_HEIGHT_LIMIT 2048
#define TEXTURE_CHANNEL_CNT 4
#define TEXTURE_PX_LIMIT (TEXTURE_WIDTH_LIMIT * TEXTURE_HEIGHT_LIMIT)
#define TEXTURE_PX_DATA_SIZE_LIMIT (TEXTURE_CHANNEL_CNT * TEXTURE_PX_LIMIT)

#define FONT_CHAR_RANGE_BEGIN 32
#define FONT_CHAR_RANGE_LEN 95

#define SHADER_SRC_LEN_LIMIT 4095

#define AUDIO_SAMPLES_PER_CHUNK 44100

#define SOUND_SAMPLE_LIMIT 441000

typedef float ta_audio_sample;

enum asset_type {
    ec_asset_type_texture,
    ec_asset_type_font,
    ec_asset_type_shader_prog,
    ec_asset_type_sound,
    ec_asset_type_music,

    ecs_asset_type_cnt
};

typedef struct {
    int hor_offsets[FONT_CHAR_RANGE_LEN];
    int ver_offsets[FONT_CHAR_RANGE_LEN];
    int hor_advances[FONT_CHAR_RANGE_LEN];

    s_rect_i src_rects[FONT_CHAR_RANGE_LEN];

    int kernings[FONT_CHAR_RANGE_LEN * FONT_CHAR_RANGE_LEN];
} s_font_chars_arrangement_info;

typedef struct {
    int line_height;
    s_font_chars_arrangement_info chars;
} s_font_arrangement_info;

typedef struct {
    int channel_cnt;
    long long sample_cnt_per_channel;
    int sample_rate;
} s_audio_info;

#endif
