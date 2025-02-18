#pragma once

#include <zf4c.h>
#include <zf4_assets.h>

namespace zf4 {
    constexpr int g_render_surface_limit = 128;

    constexpr int g_texture_batch_slot_vert_cnt = g_textured_quad_shader_prog_cnt * 4;
    constexpr int g_texture_batch_slot_verts_size = sizeof(float) * g_texture_batch_slot_vert_cnt;
    constexpr int g_texture_batch_slot_indices_cnt = 6;
    constexpr int g_texture_batch_slot_limit = 2048;

    constexpr int g_str_draw_len_limit = 256;

    namespace colors {
        constexpr s_vec_4d g_white = {1.0f, 1.0f, 1.0f, 1.0f};
        constexpr s_vec_4d g_black = {0.0f, 0.0f, 0.0f, 1.0f};
        constexpr s_vec_4d g_gray = {0.5f, 0.5f, 0.5f, 1.0f};
        constexpr s_vec_4d g_red = {1.0f, 0.0f, 0.0f, 1.0f};
        constexpr s_vec_4d g_green = {0.0f, 1.0f, 0.0f, 1.0f};
        constexpr s_vec_4d g_blue = {0.0f, 0.0f, 1.0f, 1.0f};
        constexpr s_vec_4d g_yellow = {1.0f, 1.0f, 0.0f, 1.0f};
        constexpr s_vec_4d g_cyan = {0.0f, 1.0f, 1.0f, 1.0f};
        constexpr s_vec_4d g_magenta = {1.0f, 0.0f, 1.0f, 1.0f};
        constexpr s_vec_4d g_orange = {1.0f, 0.5f, 0.0f, 1.0f};
        constexpr s_vec_4d g_purple = {0.5f, 0.0f, 0.5f, 1.0f};
        constexpr s_vec_4d g_brown = {0.6f, 0.3f, 0.0f, 1.0f};
        constexpr s_vec_4d g_pink = {1.0f, 0.75f, 0.8f, 1.0f};
        constexpr s_vec_4d g_lime = {0.75f, 1.0f, 0.0f, 1.0f};
        constexpr s_vec_4d g_olive = {0.5f, 0.5f, 0.0f, 1.0f};
        constexpr s_vec_4d g_maroon = {0.5f, 0.0f, 0.0f, 1.0f};
        constexpr s_vec_4d g_navy = {0.0f, 0.0f, 0.5f, 1.0f};
        constexpr s_vec_4d g_teal = {0.0f, 0.5f, 0.5f, 1.0f};
        constexpr s_vec_4d g_aqua = {0.0f, 1.0f, 1.0f, 1.0f};
        constexpr s_vec_4d g_silver = {0.75f, 0.75f, 0.75f, 1.0f};
        constexpr s_vec_4d g_gold = {1.0f, 0.84f, 0.0f, 1.0f};
        constexpr s_vec_4d g_beige = {0.96f, 0.96f, 0.86f, 1.0f};
        constexpr s_vec_4d g_ivory = {1.0f, 1.0f, 0.94f, 1.0f};
        constexpr s_vec_4d g_turquoise = {0.25f, 0.88f, 0.82f, 1.0f};
        constexpr s_vec_4d g_violet = {0.93f, 0.51f, 0.93f, 1.0f};
        constexpr s_vec_4d g_indigo = {0.29f, 0.0f, 0.51f, 1.0f};
        constexpr s_vec_4d g_chocolate = {0.82f, 0.41f, 0.12f, 1.0f};
        constexpr s_vec_4d g_coral = {1.0f, 0.5f, 0.31f, 1.0f};
        constexpr s_vec_4d g_salmon = {0.98f, 0.5f, 0.45f, 1.0f};
        constexpr s_vec_4d g_khaki = {0.94f, 0.9f, 0.55f, 1.0f};
        constexpr s_vec_4d g_plum = {0.87f, 0.63f, 0.87f, 1.0f};
        constexpr s_vec_4d g_sienna = {0.63f, 0.32f, 0.18f, 1.0f};
        constexpr s_vec_4d g_tan = {0.82f, 0.71f, 0.55f, 1.0f};
        constexpr s_vec_4d g_thistle = {0.85f, 0.75f, 0.85f, 1.0f};
        constexpr s_vec_4d g_wheat = {0.96f, 0.87f, 0.7f, 1.0f};
    }

    enum e_str_hor_align {
        ek_str_hor_align_left,
        ek_str_hor_align_center,
        ek_str_hor_align_right
    };

    enum e_str_ver_align {
        ek_str_ver_align_top,
        ek_str_ver_align_center,
        ek_str_ver_align_bottom
    };

    enum e_shader_uniform_val_type {
        ek_shader_uniform_val_type_int,
        ek_shader_uniform_val_type_float,
        ek_shader_uniform_val_type_v2,
        ek_shader_uniform_val_type_v3,
        ek_shader_uniform_val_type_v4,
        ek_shader_uniform_val_type_mat4x4,

        eks_shader_uniform_val_type_cnt
    };

    union u_shader_uniform_val {
        int i;
        float f;
        s_vec_2d v2;
        s_vec_3d v3;
        s_vec_4d v4;
        s_matrix_4x4 mat4x4;
    };

    struct s_str_draw_info {
        int char_cnt;
        s_vec_2d_i char_draw_positions[g_str_draw_len_limit];

        int line_cnt;
        int line_widths[g_str_draw_len_limit + 1]; // Maximised for the case where all characters are newlines.

        int height;
    };

    struct s_textured_quad_shader_prog {
        GLuint gl_id;
        int proj_uniform_loc;
        int view_uniform_loc;
        int textures_uniform_loc;
    };

    struct s_render_surfaces {
        s_static_array<GLuint, g_render_surface_limit> framebuffer_gl_ids;
        s_static_array<GLuint, g_render_surface_limit> framebuffer_tex_gl_ids;

        int cnt;
    };

    struct s_pers_render_data {
        s_textured_quad_shader_prog textured_quad_shader_prog;

        s_render_surfaces surfs;
        GLuint surf_vert_array_gl_id;
        GLuint surf_vert_buf_gl_id;
        GLuint surf_elem_buf_gl_id;

        GLuint tex_batch_vert_array_gl_id;
        GLuint tex_batch_vert_buf_gl_id;
        GLuint tex_batch_elem_buf_gl_id;
    };

    struct s_draw_phase_state {
        GLuint surf_shader_prog_gl_id; // When we draw a surface, it will use this shader program.

        s_static_list<int, g_render_surface_limit> surf_index_stack;

        s_static_array<s_static_array<float, g_texture_batch_slot_vert_cnt>, g_texture_batch_slot_limit> tex_batch_slot_verts;
        int tex_batch_slots_used_cnt;
        GLuint tex_batch_tex_gl_id; // NOTE: In the future we will support multiple texture units. This is just for simplicity right now.

        s_matrix_4x4 proj_mat;
        s_matrix_4x4 view_mat;
    };

    s_pers_render_data* LoadPersRenderData(s_mem_arena& mem_arena, s_mem_arena& scratch_space);
    void CleanPersRenderData(s_pers_render_data& pers_render_data);

    bool InitRenderSurfaces(const int cnt, s_render_surfaces& surfs, const s_vec_2d_i window_size);
    void CleanRenderSurfaces(s_render_surfaces& surfs);
    bool ResizeRenderSurfaces(s_render_surfaces& surfs, const s_vec_2d_i window_size);

    s_draw_phase_state* BeginDrawPhase(s_mem_arena& mem_arena, const s_vec_2d_i window_size);
    void RenderClear(const s_vec_4d col = {});
    void SetRenderSurface(const int surf_index, s_draw_phase_state& draw_phase_state, const s_pers_render_data& pers_render_data);
    void UnsetRenderSurface(s_draw_phase_state& draw_phase_state, const s_pers_render_data& pers_render_data);
    void SetRenderSurfaceShaderProg(const int prog_index, const s_shader_progs& progs, s_draw_phase_state& draw_phase_state);
    void SetRenderSurfaceShaderProgUniform(const char* const uni_name, const u_shader_uniform_val val, const e_shader_uniform_val_type val_type, s_draw_phase_state& draw_phase_state);
    void DrawRenderSurface(const int surf_index, s_draw_phase_state& draw_phase_state, const s_pers_render_data& pers_render_data);
    void SubmitToRenderBatch(const GLuint tex_gl_id, const s_rect_edges tex_coords, const s_vec_2d pos, const s_vec_2d size, s_draw_phase_state& draw_phase_state, const s_pers_render_data& pers_render_data, const s_vec_2d origin = {0.5f, 0.5f}, const float rot = 0.0f, const s_vec_4d blend = colors::g_white);
    void SubmitTextureToRenderBatch(const int tex_index, const s_textures& textures, const s_rect_i src_rect, const s_vec_2d pos, s_draw_phase_state& draw_phase_state, const s_pers_render_data& pers_render_data, const s_vec_2d origin = {0.5f, 0.5f}, const s_vec_2d scale = {1.0f, 1.0f}, const float rot = 0.0f, const s_vec_4d blend = {1.0f, 1.0f, 1.0f, 1.0f});
    void SubmitStrToRenderBatch(const char* const str, const int font_index, const s_fonts& fonts, const s_vec_2d pos, const s_vec_4d blend, const e_str_hor_align hor_align, const e_str_ver_align ver_align, s_draw_phase_state& draw_phase_state, const s_pers_render_data& pers_render_data);
    void FlushRenderBatch(s_draw_phase_state& draw_phase_state, const s_pers_render_data& pers_render_data);

    s_str_draw_info LoadStrDrawInfo(const char* const str, const int font_index, const s_fonts& fonts);

    s_rect_edges CalcTexCoords(const s_rect_i src_rect, const s_vec_2d_i tex_size);

    void DrawRect(const s_rect rect, const s_vec_4d blend, s_draw_phase_state& draw_phase_state, const s_pers_render_data& pers_render_data, const s_builtin_assets& builtin_assets);
    void DrawRectOutline(const s_rect rect, const s_vec_4d blend, s_draw_phase_state& draw_phase_state, const s_pers_render_data& pers_render_data, const s_builtin_assets& builtin_assets);
    void DrawLine(const s_vec_2d start, const s_vec_2d end, const float width, const s_vec_4d blend, s_draw_phase_state& draw_phase_state, const s_pers_render_data& pers_render_data, const s_builtin_assets& builtin_assets);
    void DrawBar(const s_vec_2d pos, const s_vec_2d size, const float perc, const s_vec_3d col_front, const s_vec_3d col_back, const float alpha, s_draw_phase_state& draw_phase_state, const s_pers_render_data& pers_render_data, const s_builtin_assets& builtin_assets);

    inline void SetRenderSurfaceShaderProgUniformInt(const char* const uni_name, const int val, s_draw_phase_state& draw_phase_state) {
        const u_shader_uniform_val uni_val = {.i = val};
        SetRenderSurfaceShaderProgUniform(uni_name, uni_val, ek_shader_uniform_val_type_int, draw_phase_state);
    }

    inline void SetRenderSurfaceShaderProgUniformFloat(const char* const uni_name, const float val, s_draw_phase_state& draw_phase_state) {
        const u_shader_uniform_val uni_val = {.f = val};
        SetRenderSurfaceShaderProgUniform(uni_name, uni_val, ek_shader_uniform_val_type_float, draw_phase_state);
    }

    inline void SetRenderSurfaceShaderProgUniformVec2D(const char* const uni_name, const s_vec_2d val, s_draw_phase_state& draw_phase_state) {
        const u_shader_uniform_val uni_val = {.v2 = val};
        SetRenderSurfaceShaderProgUniform(uni_name, uni_val, ek_shader_uniform_val_type_v2, draw_phase_state);
    }

    inline void SetRenderSurfaceShaderProgUniformVec3D(const char* const uni_name, const s_vec_3d val, s_draw_phase_state& draw_phase_state) {
        const u_shader_uniform_val uni_val = {.v3 = val};
        SetRenderSurfaceShaderProgUniform(uni_name, uni_val, ek_shader_uniform_val_type_v3, draw_phase_state);
    }

    inline void SetRenderSurfaceShaderProgUniformVec4D(const char* const uni_name, const s_vec_4d val, s_draw_phase_state& draw_phase_state) {
        const u_shader_uniform_val uni_val = {.v4 = val};
        SetRenderSurfaceShaderProgUniform(uni_name, uni_val, ek_shader_uniform_val_type_v4, draw_phase_state);
    }

    inline void SetRenderSurfaceShaderProgUniformMat4x4(const char* const uni_name, const s_matrix_4x4 val, s_draw_phase_state& draw_phase_state) {
        const u_shader_uniform_val uni_val = {.mat4x4 = val};
        SetRenderSurfaceShaderProgUniform(uni_name, uni_val, ek_shader_uniform_val_type_mat4x4, draw_phase_state);
    }
}
