#pragma once

#include <glad/glad.h>
#include <AL/al.h>
#include <zf4c.h>

namespace zf4 {
    constexpr int g_textured_quad_shader_prog_cnt = 13;

    constexpr int g_texture_limit = 64;
    constexpr int g_font_limit = 24;
    constexpr int g_shader_prog_limit = 24;
    constexpr int g_sound_limit = 24;
    constexpr int g_music_limit = 24;

    struct s_textures {
        s_static_array<GLuint, g_texture_limit> gl_ids;
        s_static_array<s_vec_2d_i, g_texture_limit> sizes;
        int cnt;
    };

    struct s_fonts {
        s_static_array<s_font_arrangement_info, g_font_limit> arrangement_infos;

        s_static_array<GLuint, g_font_limit> tex_gl_ids;
        s_static_array<s_vec_2d_i, g_font_limit> tex_sizes;

        int cnt;
    };

    struct s_shader_progs {
        s_static_array<GLuint, g_shader_prog_limit> gl_ids;
        int cnt;
    };

    struct s_sounds {
        s_static_array<ALuint, g_sound_limit> buf_al_ids;
        int cnt;
    };

    struct s_music {
        s_static_array<s_audio_info, g_music_limit> infos;
        s_static_array<int, g_music_limit> sample_data_file_positions;
        int cnt;
    };

    struct s_assets {
        s_textures textures;
        s_fonts fonts;
        s_shader_progs shader_progs;
        s_sounds sounds;
        s_music music;
    };

    s_assets* LoadAssets(s_mem_arena& mem_arena, s_mem_arena& scratch_space);
    void UnloadAssets(s_assets& assets);

    GLuint CreateShaderFromSrc(const char* const src, const bool frag);
    GLuint CreateShaderProgFromSrcs(const char* const vert_shader_src, const char* const frag_shader_src);
}
