#pragma once

#include <glad/glad.h>
#include <AL/al.h>
#include <zf4c.h>

namespace zf4 {
    constexpr int g_textured_quad_shader_prog_cnt = 13;

    struct s_textures {
        s_array<const GLuint> gl_ids;
        s_array<const s_vec_2d_i> sizes;
        int cnt;
    };

    struct s_fonts {
        s_array<const s_font_arrangement_info> arrangement_infos;

        s_array<const GLuint> tex_gl_ids;
        s_array<const s_vec_2d_i> tex_sizes;

        int cnt;
    };

    struct s_shader_progs {
        s_array<const GLuint> gl_ids;
        int cnt;
    };

    struct s_sounds {
        s_array<const ALuint> buf_al_ids;
        int cnt;
    };

    struct s_music {
        s_array<const s_audio_info> infos;
        s_array<const int> sample_data_file_positions;
        int cnt;
    };

    struct s_user_assets {
        s_textures textures;
        s_fonts fonts;
        s_shader_progs shader_progs;
        s_sounds sounds;
        s_music music;
    };

    enum e_builtin_texture {
        ek_builtin_texture_pixel,

        eks_builtin_texture_cnt
    };

    enum e_builtin_shader_prog {
        ek_builtin_shader_prog_blend,
        ek_builtin_shader_prog_outline,
        ek_builtin_shader_prog_circle,

        eks_builtin_shader_prog_cnt
    };

    struct s_builtin_assets {
        GLuint pixel_tex_gl_id;

        GLuint blend_shader_prog_gl_id;
        //GLuint outline_shader_prog_gl_id;
        //GLuint circle_shader_prog_gl_id;
    };

    bool LoadUserAssets(s_user_assets& assets, s_mem_arena& mem_arena, s_mem_arena& scratch_space);
    void UnloadUserAssets(s_user_assets& assets);

    s_builtin_assets LoadBuiltinAssets();
    void UnloadBuiltinAssets(s_builtin_assets& assets);

    void SetUpGeneratedGLTexture(const GLuint gl_id, const s_vec_2d_i size, const s_array<const unsigned char> px_data);

    GLuint CreateShaderFromSrc(const char* const src, const bool frag);
    GLuint CreateShaderProgFromSrcs(const char* const vert_shader_src, const char* const frag_shader_src);
}
