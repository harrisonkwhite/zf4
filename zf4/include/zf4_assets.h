#ifndef ZF4_ASSETS_H
#define ZF4_ASSETS_H

#include <stdio.h>
#include <glad/glad.h>
#include <AL/al.h>
#include <zf4c.h>

#define TEXTURED_QUAD_SHADER_PROG_VERT_CNT 13

#define TEXTURE_LIMIT 64
#define FONT_LIMIT 24
#define SHADER_PROG_LIMIT 24
#define SOUND_LIMIT 24
#define MUSIC_LIMIT 24

typedef struct textures {
    GLuint gl_ids[TEXTURE_LIMIT];
    s_vec_2d_i sizes[TEXTURE_LIMIT];

    int cnt;
} s_textures;

typedef struct fonts {
    s_font_arrangement_info arrangement_infos[FONT_LIMIT];

    GLuint tex_gl_ids[FONT_LIMIT];
    s_vec_2d_i tex_sizes[FONT_LIMIT];

    int cnt;
} s_fonts;

typedef struct shader_progs {
    GLuint gl_ids[SHADER_PROG_LIMIT];
    int cnt;
} s_shader_progs;

typedef struct sounds {
    ALuint buf_al_ids[SOUND_LIMIT];
    int cnt;
} s_sounds;

typedef struct music {
    s_audio_info infos[MUSIC_LIMIT];
    int sample_data_file_positions[MUSIC_LIMIT];

    int cnt;
} s_music;

typedef struct assets {
    s_textures textures;
    s_fonts fonts;
    s_shader_progs shader_progs;
    s_sounds sounds;
    s_music music;
} s_assets;

s_assets* LoadAssets(s_mem_arena* const mem_arena, s_mem_arena* const scratch_space);
void UnloadAssets(s_assets* const assets);

GLuint CreateShaderFromSrc(const char* const src, const bool frag);
GLuint CreateShaderProgFromSrcs(const char* const vert_shader_src, const char* const frag_shader_src);

#endif
