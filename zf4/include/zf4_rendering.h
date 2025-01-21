#ifndef ZF4_RENDERING_H
#define ZF4_RENDERING_H

#include <zf4c.h>
#include <zf4_assets.h>

#define COLOR_WHITE V4(1.0f, 1.0f, 1.0f, 1.0f)
#define COLOR_BLACK V4(0.0f, 0.0f, 0.0f, 1.0f)
#define COLOR_TRANSPARENT_BLACK V4(0.0f, 0.0f, 0.0f, 0.0f)

#define RENDER_SURFACE_LIMIT 128

#define TEXTURE_BATCH_SLOT_VERT_CNT (TEXTURED_QUAD_SHADER_PROG_VERT_CNT * 4)
#define TEXTURE_BATCH_SLOT_VERTS_SIZE (sizeof(float) * TEXTURE_BATCH_SLOT_VERT_CNT)
#define TEXTURE_BATCH_SLOT_INDICES_CNT 6
#define TEXTURE_BATCH_SLOT_LIMIT 2048

#define STR_DRAW_LEN_LIMIT 256 // NOTE: A bit arbitrary, could remove.

enum str_hor_align {
    str_hor_align__left,
    str_hor_align__center,
    str_hor_align__right
};

enum str_ver_align {
    str_ver_align__top,
    str_ver_align__center,
    str_ver_align__bottom
};

enum shader_uniform_val_type {
    ev_shader_uniform_val_type__int,
    ev_shader_uniform_val_type__float,
    ev_shader_uniform_val_type__vec2d,
    ev_shader_uniform_val_type__vec3d,
    ev_shader_uniform_val_type__vec4d,
    ev_shader_uniform_val_type__mat4x4,

    shader_uniform_val_type_cnt
};

typedef union {
    int i;
    float f;
    s_vec_2d v2;
    s_vec_3d v3;
    s_vec_4d v4;
    s_matrix_4x4 m4x4;
} u_shader_uniform_val;

typedef struct {
    int char_cnt;
    s_vec_2d_i char_draw_positions[STR_DRAW_LEN_LIMIT];

    int line_cnt;
    int line_widths[STR_DRAW_LEN_LIMIT + 1]; // Maximised for the case where all characters are newlines.

    int height;
} s_str_draw_info;

typedef struct {
    GLuint gl_id;
    int proj_uniform_loc;
    int view_uniform_loc;
    int textures_uniform_loc;
} s_textured_quad_shader_prog;

typedef struct {
    GLuint framebuffer_gl_ids[RENDER_SURFACE_LIMIT];
    GLuint framebuffer_tex_gl_ids[RENDER_SURFACE_LIMIT];
    int cnt;
} s_render_surfaces;

typedef struct {
    s_textured_quad_shader_prog textured_quad_shader_prog;

    s_render_surfaces surfs;
    GLuint surf_vert_array_gl_id;
    GLuint surf_vert_buf_gl_id;
    GLuint surf_elem_buf_gl_id;

    GLuint tex_batch_vert_array_gl_id;
    GLuint tex_batch_vert_buf_gl_id;
    GLuint tex_batch_elem_buf_gl_id;
} s_renderer;

typedef struct {
    GLuint surf_shader_prog_gl_id; // When we draw a surface, it will use this shader program.

    int surf_index_stack[RENDER_SURFACE_LIMIT];
    int surf_index_stack_height;

    float tex_batch_slot_verts[TEXTURE_BATCH_SLOT_LIMIT][TEXTURE_BATCH_SLOT_VERT_CNT];
    int tex_batch_slots_used_cnt;
    GLuint tex_batch_tex_gl_id; // NOTE: In the future we will support multiple texture units. This is just for simplicity right now.

    s_matrix_4x4 proj_mat;
    s_matrix_4x4 view_mat;

    const s_assets* assets;
} s_draw_phase_state;

s_renderer* LoadRenderer(s_mem_arena* const mem_arena, s_mem_arena* const scratch_space);
void CleanRenderer(s_renderer* const renderer);

bool InitRenderSurfaces(const int cnt, s_render_surfaces* const surfs, const s_vec_2d_i window_size);
void CleanRenderSurfaces(s_render_surfaces* const surfs);
bool ResizeRenderSurfaces(s_render_surfaces* const surfs, const s_vec_2d_i window_size);

s_draw_phase_state* BeginDrawPhase(s_mem_arena* const mem_arena, const s_vec_2d_i window_size, const s_assets* const assets);
void RenderClear(const s_vec_4d col);
void SetRenderSurface(const int surf_index, s_draw_phase_state* const draw_phase_state, const s_renderer* const renderer);
void UnsetRenderSurface(s_draw_phase_state* const draw_phase_state, const s_renderer* const renderer);
void SetRenderSurfaceShaderProg(const int shader_prog_index, s_draw_phase_state* const draw_phase_state);
void SetRenderSurfaceShaderProgUniform(const char* const uni_name, const u_shader_uniform_val val, const enum shader_uniform_val_type val_type, s_draw_phase_state* const draw_phase_state);
void DrawRenderSurface(const int surf_index, s_draw_phase_state* const draw_phase_state, const s_renderer* const renderer);
void SubmitTextureToRenderBatch(const int tex_index, const s_rect_i src_rect, const s_vec_2d pos, const s_vec_2d origin, const s_vec_2d scale, const float rot, const s_vec_4d blend, s_draw_phase_state* const draw_phase_state, const s_renderer* const renderer);
void SubmitStrToRenderBatch(const char* const str, const int font_index, const s_vec_2d pos, const s_vec_4d blend, const enum str_hor_align hor_align, const enum str_ver_align ver_align, s_draw_phase_state* const draw_phase_state, const s_renderer* const renderer);
void FlushTextureBatch(s_draw_phase_state* const draw_phase_state, const s_renderer* const renderer);

inline bool IsColorValid(const s_vec_4d* const col) {
    return col->x >= 0.0f && col->x <= 1.0f
        && col->y >= 0.0f && col->y <= 1.0f
        && col->z >= 0.0f && col->z <= 1.0f
        && col->w >= 0.0f && col->w <= 1.0f;
}

inline bool IsSrcRectValid(const s_rect_i* const src_rect, const s_vec_2d_i tex_size) {
    return src_rect->x >= 0 && src_rect->y >= 0 && src_rect->width > 0 && src_rect->height > 0 && src_rect->x + src_rect->width <= tex_size.x && src_rect->y + src_rect->height <= tex_size.y;
}

inline void RenderSurfacesValidator(const s_render_surfaces* const surfs) {
    assert(surfs);
    assert(surfs->cnt >= 0 && surfs->cnt <= RENDER_SURFACE_LIMIT);
}

inline void SetRenderSurfaceShaderProgUniformInt(const char* const uni_name, const int val, s_draw_phase_state* const draw_phase_state) {
    const u_shader_uniform_val uni_val = {.i = val};
    SetRenderSurfaceShaderProgUniform(uni_name, uni_val, ev_shader_uniform_val_type__int, draw_phase_state);
}

inline void SetRenderSurfaceShaderProgUniformFloat(const char* const uni_name, const float val, s_draw_phase_state* const draw_phase_state) {
    const u_shader_uniform_val uni_val = {.f = val};
    SetRenderSurfaceShaderProgUniform(uni_name, uni_val, ev_shader_uniform_val_type__float, draw_phase_state);
}

inline void SetRenderSurfaceShaderProgUniformVec2D(const char* const uni_name, const s_vec_2d val, s_draw_phase_state* const draw_phase_state) {
    const u_shader_uniform_val uni_val = {.v2 = val};
    SetRenderSurfaceShaderProgUniform(uni_name, uni_val, ev_shader_uniform_val_type__vec2d, draw_phase_state);
}

inline void SetRenderSurfaceShaderProgUniformVec3D(const char* const uni_name, const s_vec_3d val, s_draw_phase_state* const draw_phase_state) {
    const u_shader_uniform_val uni_val = {.v3 = val};
    SetRenderSurfaceShaderProgUniform(uni_name, uni_val, ev_shader_uniform_val_type__vec3d, draw_phase_state);
}

inline void SetRenderSurfaceShaderProgUniformVec4D(const char* const uni_name, const s_vec_4d val, s_draw_phase_state* const draw_phase_state) {
    const u_shader_uniform_val uni_val = {.v4 = val};
    SetRenderSurfaceShaderProgUniform(uni_name, uni_val, ev_shader_uniform_val_type__vec4d, draw_phase_state);
}

inline void SetRenderSurfaceShaderProgUniformMat4x4(const char* const uni_name, const s_matrix_4x4 val, s_draw_phase_state* const draw_phase_state) {
    const u_shader_uniform_val uni_val = {.m4x4 = val};
    SetRenderSurfaceShaderProgUniform(uni_name, uni_val, ev_shader_uniform_val_type__mat4x4, draw_phase_state);
}

#endif
