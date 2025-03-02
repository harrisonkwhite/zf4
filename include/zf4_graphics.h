#pragma once

#include <cassert>
#include <glad/glad.h>
#include <zf4_math.h>

#define GL_CALL(X) X; assert(glGetError() == GL_NO_ERROR)

namespace zf4::graphics {
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

    constexpr int g_font_char_range_begin = 32;
    constexpr int g_font_char_range_len = 95;

    constexpr int g_str_draw_len_limit = 256;

    constexpr int g_shader_uniform_name_len_limit = 32; // TEMP? Kind of arbitrary.

    constexpr int g_surface_limit = 128;

    using a_gl_id = GLuint;

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

    struct s_textures {
        s_array<const a_gl_id> gl_ids;
        s_array<const s_vec_2d_i> sizes;
        int cnt;
    };

    struct s_font_chars_arrangement_info {
        s_static_array<int, g_font_char_range_len> hor_offsets;
        s_static_array<int, g_font_char_range_len> ver_offsets;
        s_static_array<int, g_font_char_range_len> hor_advances;

        s_static_array<s_rect_i, g_font_char_range_len> src_rects;
    };

    struct s_font_arrangement_info {
        int line_height;
        s_font_chars_arrangement_info chars;
    };

    struct s_fonts {
        s_array<const s_font_arrangement_info> arrangement_infos;

        s_array<const a_gl_id> tex_gl_ids;
        s_array<const s_vec_2d_i> tex_sizes;

        int cnt;
    };

    enum e_instr_type {
        ek_instr_type_clear,
        ek_instr_type_set_view_matrix,
        ek_instr_type_draw_texture,
        ek_instr_type_set_surf,
        ek_instr_type_unset_surf,
        ek_instr_type_set_surf_shader_prog_uniform,
        ek_instr_type_draw_surf
    };

    struct s_clear_instr {
        s_vec_4d color;
    };

    struct s_set_view_matrix_instr {
        s_matrix_4x4 mat;
    };

    struct s_draw_texture_instr {
        a_gl_id tex_gl_id;
        s_rect_edges tex_coords;
        s_vec_2d pos;
        s_vec_2d size;
        s_vec_2d origin;
        float rot;
        s_vec_4d blend;
    };

    struct s_set_surf_instr {
        int surf_index;
    };

    struct s_draw_surf_instr {
        int surf_index;
    };

    enum e_shader_uniform_val_type {
        ek_shader_uniform_val_type_int,
        ek_shader_uniform_val_type_float,
        ek_shader_uniform_val_type_v2,
        ek_shader_uniform_val_type_v3,
        ek_shader_uniform_val_type_v4,
        ek_shader_uniform_val_type_mat_4x4,

        eks_shader_uniform_val_type_cnt
    };

    struct s_set_surf_shader_prog_uniform_instr {
        char name[g_shader_uniform_name_len_limit];
        e_shader_uniform_val_type val_type;

        union {
            int val_as_int;
            float val_as_float;
            s_vec_2d val_as_v2;
            s_vec_3d val_as_v3;
            s_vec_4d val_as_v4;
            s_matrix_4x4 val_as_mat_4x4;
        };
    };

    struct s_draw_instr {
        e_instr_type type;

        union {
            s_clear_instr clear;
            s_set_view_matrix_instr set_view_mat;
            s_draw_texture_instr draw_tex;
            s_set_surf_instr set_surf;
            s_draw_surf_instr draw_surf;
            s_set_surf_shader_prog_uniform_instr set_surf_shader_prog_uni;
        };
    };

    struct s_textured_quad_shader_prog {
        a_gl_id gl_id;
        int proj_uniform_loc;
        int view_uniform_loc;
        int textures_uniform_loc;
    };

    struct s_batch {
        a_gl_id vert_array_gl_id;
        a_gl_id vert_buf_gl_id;
        a_gl_id elem_buf_gl_id;
    };

    struct s_surf_pers_data { // TODO: Rename!
        a_gl_id vert_array_gl_id;
        a_gl_id vert_buf_gl_id;
        a_gl_id elem_buf_gl_id;
    };

    struct s_pers_render_data {
        s_textured_quad_shader_prog textured_quad_shader_prog;
        s_batch batch;
        s_surf_pers_data surf_pers_data;
    };

    struct s_surfaces {
        s_static_array<a_gl_id, g_surface_limit> framebuffer_gl_ids;
        s_static_array<a_gl_id, g_surface_limit> framebuffer_tex_gl_ids;

        int cnt;
    };

    struct s_str_draw_info {
        int char_cnt;
        s_vec_2d_i char_draw_positions[g_str_draw_len_limit];

        int line_cnt;
        int line_widths[g_str_draw_len_limit + 1]; // Maximised for the case where all characters are newlines.

        int height;
    };

    s_textures PushTextures(const int cnt, const s_array<const char* const> filenames, s_mem_arena& mem_arena, bool& err);
    void UnloadTextures(const s_textures& textures);

    s_fonts PushFonts(const int cnt, const s_array<const char* const> filenames, const s_array<const int> pt_sizes, s_mem_arena& mem_arena, s_mem_arena& scratch_space, bool& err);
    void UnloadFonts(const s_fonts& fonts);

    s_pers_render_data GenPersRenderData();
    void CleanPersRenderData(const s_pers_render_data& render_data);

    bool InitSurfaces(const int cnt, s_surfaces& surfs, const s_vec_2d_i window_size);
    void CleanAndZeroOutSurfaces(s_surfaces& surfs);
    bool ResizeSurfaces(s_surfaces& surfs, const s_vec_2d_i window_size);

    bool ExecDrawInstrs(const s_array<const s_draw_instr> instrs, const s_vec_2d_i window_size, const s_pers_render_data& pers_render_data, const s_surfaces& surfs, const s_textures& textures, s_mem_arena& scratch_space);
    bool AppendDrawTextureInstr(s_list<s_draw_instr>& instrs, const int tex_index, const s_textures& textures, const s_rect_i src_rect, const s_vec_2d pos, const s_vec_2d origin = {0.5f, 0.5f}, const s_vec_2d scale = {1.0f, 1.0f}, const float rot = 0.0f, const s_vec_4d blend = {1.0f, 1.0f, 1.0f, 1.0f});
    bool AppendDrawTextureInstrsForStr(s_list<s_draw_instr>& instrs, const char* const str, const int font_index, const s_fonts& fonts, const s_vec_2d pos, const s_vec_4d blend, const e_str_hor_align hor_align, const e_str_ver_align ver_align);

    s_str_draw_info LoadStrDrawInfo(const char* const str, const int font_index, const s_fonts& fonts);

    inline bool AppendClearInstr(s_list<s_draw_instr>& instrs, const s_vec_4d color = {}) {
        return ListAppendTry(instrs, {
            .type = ek_instr_type_clear,
            .clear = {.color = color}
        });
    }

    inline bool AppendSetViewMatrixInstr(s_list<s_draw_instr>& instrs, const s_matrix_4x4& mat) {
        return ListAppendTry(instrs, {
            .type = ek_instr_type_set_view_matrix,
            .set_view_mat = {.mat = mat}
        });
    }

    inline bool AppendDrawTextureInstrsForRect(s_list<s_draw_instr>& instrs, const s_rect rect, const s_vec_4d blend, const int px_tex_index, const s_textures& textures, const s_rect_i px_src_rect) {
        return AppendDrawTextureInstr(instrs, px_tex_index, textures, px_src_rect, RectTopLeft(rect), {}, RectSize(rect), 0.0f, blend);
    }

    inline bool AppendSetSurfInstr(s_list<s_draw_instr>& instrs, const int surf_index) {
        return ListAppendTry(instrs, {
            .type = ek_instr_type_set_surf,
            .set_surf = {.surf_index = surf_index}
        });
    }

    inline bool AppendUnsetSurfInstr(s_list<s_draw_instr>& instrs) {
        return ListAppendTry(instrs, {
            .type = ek_instr_type_unset_surf
        });
    }

    inline bool AppendSetSurfaceShaderProgUniformInstr(s_list<s_draw_instr>& instrs, const char* const uni_name, const int val) {
        return ListAppendTry(instrs, {
            .type = ek_instr_type_set_surf_shader_prog_uniform,
            .set_surf_shader_prog_uni = {.val_type = ek_shader_uniform_val_type_int, .val_as_int = val}
        });
    }

    inline bool AppendSetSurfaceShaderProgUniformInstr(s_list<s_draw_instr>& instrs, const char* const uni_name, const float val) {
        return ListAppendTry(instrs, {
            .type = ek_instr_type_set_surf_shader_prog_uniform,
            .set_surf_shader_prog_uni = {.val_type = ek_shader_uniform_val_type_float, .val_as_float = val}
        });
    }

    inline bool AppendSetSurfaceShaderProgUniformInstr(s_list<s_draw_instr>& instrs, const char* const uni_name, const s_vec_2d val) {
        return ListAppendTry(instrs, {
            .type = ek_instr_type_set_surf_shader_prog_uniform,
            .set_surf_shader_prog_uni = {.val_type = ek_shader_uniform_val_type_v2, .val_as_v2 = val}
        });
    }

    inline bool AppendSetSurfaceShaderProgUniformInstr(s_list<s_draw_instr>& instrs, const char* const uni_name, const s_vec_3d val) {
        return ListAppendTry(instrs, {
            .type = ek_instr_type_set_surf_shader_prog_uniform,
            .set_surf_shader_prog_uni = {.val_type = ek_shader_uniform_val_type_v3, .val_as_v3 = val}
        });
    }

    inline bool AppendSetSurfaceShaderProgUniformInstr(s_list<s_draw_instr>& instrs, const char* const uni_name, const s_vec_4d val) {
        return ListAppendTry(instrs, {
            .type = ek_instr_type_set_surf_shader_prog_uniform,
            .set_surf_shader_prog_uni = {.val_type = ek_shader_uniform_val_type_v4, .val_as_v4 = val}
        });
    }

    inline bool AppendSetSurfaceShaderProgUniformInstr(s_list<s_draw_instr>& instrs, const char* const uni_name, const s_matrix_4x4 val) {
        return ListAppendTry(instrs, {
            .type = ek_instr_type_set_surf_shader_prog_uniform,
            .set_surf_shader_prog_uni = {.val_type = ek_shader_uniform_val_type_mat_4x4, .val_as_mat_4x4 = val}
        });
    }

    inline bool AppendDrawSurfInstr(s_list<s_draw_instr>& instrs, const int surf_index) {
        return ListAppendTry(instrs, {
            .type = ek_instr_type_draw_surf,
            .draw_surf = {.surf_index = surf_index}
        });
    }
}
