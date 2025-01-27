#pragma once

#include <zf4c_math.h>

namespace zf4 {
    const char* const g_assets_file_name = "assets.dat";

    constexpr s_vec_2d_i g_texture_size_limit = {2048, 2048};
    constexpr int g_texture_channel_cnt = 4;
    constexpr int g_texture_px_limit = g_texture_size_limit.x * g_texture_size_limit.y;
    constexpr int g_texture_px_data_size_limit = g_texture_channel_cnt * g_texture_px_limit;

    constexpr int g_font_char_range_begin = 32;
    constexpr int g_font_char_range_len = 95;

    constexpr int g_shader_src_len_limit = 4095;

    constexpr int g_audio_samples_per_chunk = 44100;

    constexpr int g_sound_sample_limit = 441000;

    typedef float ta_audio_sample;

    enum e_asset_type {
        ek_asset_type_texture,
        ek_asset_type_font,
        ek_asset_type_shader_prog,
        ek_asset_type_sound,
        ek_asset_type_music,

        eks_asset_type_cnt
    };

    struct s_font_chars_arrangement_info {
        s_static_array<int, g_font_char_range_len> hor_offsets;
        s_static_array<int, g_font_char_range_len> ver_offsets;
        s_static_array<int, g_font_char_range_len> hor_advances;

        s_static_array<s_rect_i, g_font_char_range_len> src_rects;

        s_static_array<int, g_font_char_range_len* g_font_char_range_len> kernings;
    };

    struct s_font_arrangement_info {
        int line_height;
        s_font_chars_arrangement_info chars;
    };

    struct s_audio_info {
        int channel_cnt;
        long long sample_cnt_per_channel;
        int sample_rate;
    };
}
