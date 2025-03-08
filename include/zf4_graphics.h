#pragma once

#include <variant>
#include <cassert>
#include <glad/glad.h>
#include <zf4_math.h>

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

    constexpr int g_str_draw_info_str_buf_size = 256;

    constexpr int g_shader_uniform_name_buf_size = 32; // TEMP? Kind of arbitrary.

    constexpr int g_surface_limit = 128;

    using a_gl_id = GLuint;

    using a_tex_index_to_file_path_mapper = const char* (*)(const size_t index);

    struct s_font_info {
        const char* file_path = nullptr;
        size_t height = 0;

        s_font_info() = default;
        s_font_info(const char* const fp, const size_t height) : file_path(fp), height(height) {}
    };

    using a_font_index_to_info_mapper = s_font_info(*)(const size_t index);

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

    struct s_str_line_draw_info {
        int begin_chr_index = 0;
        float width_including_offs = 0.0f;
    };

    struct s_str_draw_info {
        s_static_list<s_rect, g_str_draw_info_str_buf_size> chr_draw_rects;
        s_static_list<s_str_line_draw_info, g_str_draw_info_str_buf_size> line_infos; // Maximised for the case where all characters are newlines.
    };

    struct s_clear_instr {
        s_vec_4d color;
    };

    struct s_set_view_matrix_instr {
        s_matrix_4x4 mat;
    };

    struct s_draw_texture_instr {
        a_gl_id tex_gl_id = 0;
        s_rect_edges tex_coords;
        s_vec_2d pos;
        s_vec_2d size;
        s_vec_2d origin;
        float rot = 0.0f;
        s_vec_4d blend;
    };

    struct s_set_surf_instr {
        size_t surf_index = 0;
    };

    struct s_unset_surf_instr {
    };

    struct s_draw_surf_instr {
        size_t surf_index = 0;
    };

    struct s_set_surf_shader_prog_uniform_instr {
        char name[g_shader_uniform_name_buf_size];
        std::variant<int, float, s_vec_2d, s_vec_3d, s_vec_4d, s_matrix_4x4> val;
    };

    using a_draw_instr = std::variant<s_clear_instr, s_set_view_matrix_instr, s_draw_texture_instr, s_set_surf_instr, s_unset_surf_instr, s_draw_surf_instr, s_set_surf_shader_prog_uniform_instr>;

    struct s_textures {
        s_array<const a_gl_id> gl_ids;
        s_array<const s_vec_2d_i> sizes;
        size_t cnt = 0;

        bool IsValid() const {
            if (cnt != gl_ids.len || cnt != sizes.len) {
                return false;
            }

            return gl_ids.IsValid() && sizes.IsValid();
        }
    };

    struct s_font_chars_arrangement_info {
        s_static_array<int, g_font_char_range_len> hor_offsets;
        s_static_array<int, g_font_char_range_len> ver_offsets;
        s_static_array<int, g_font_char_range_len> hor_advances;

        s_static_array<s_rect_i, g_font_char_range_len> src_rects;
    };

    struct s_font_arrangement_info {
        size_t line_height = 0;
        s_font_chars_arrangement_info chrs;
    };

    struct s_fonts {
        s_array<const s_font_arrangement_info> arrangement_infos;

        s_array<const a_gl_id> tex_gl_ids;
        s_array<const size_t> tex_heights;

        size_t cnt = 0;

        bool IsValid() const {
            if (cnt != arrangement_infos.len || cnt != tex_gl_ids.len || cnt != tex_heights.len) {
                return false;
            }

            return arrangement_infos.IsValid() && tex_gl_ids.IsValid() && tex_heights.IsValid();
        }
    };

    struct s_textured_quad_shader_prog {
        a_gl_id gl_id = 0;
        int proj_uniform_loc = 0;
        int view_uniform_loc = 0;
        int textures_uniform_loc = 0;
    };

    struct s_gl_ids {
        a_gl_id vert_array_gl_id = 0;
        a_gl_id vert_buf_gl_id = 0;
        a_gl_id elem_buf_gl_id = 0;
    };

    struct s_pers_render_data {
        s_textures textures;
        s_fonts fonts;
        s_textured_quad_shader_prog textured_quad_shader_prog;
        s_gl_ids batch_gl_ids;
        s_gl_ids surf_gl_ids;

        bool Init(const size_t tex_cnt, const a_tex_index_to_file_path_mapper tex_index_to_file_path_mapper, const size_t font_cnt, const a_font_index_to_info_mapper font_index_to_info_mapper, s_mem_arena& mem_arena, s_mem_arena& scratch_space);
        void Clean();

        bool IsValid() const {
            return textures.IsValid() && fonts.IsValid();
        }
    };

    struct s_surfaces {
        s_static_array<a_gl_id, g_surface_limit> framebuffer_gl_ids;
        s_static_array<a_gl_id, g_surface_limit> framebuffer_tex_gl_ids;

        size_t cnt = 0;

        bool Init(const int cnt, const s_vec_2d_i size);
        void Clean();
        bool Resize(const s_vec_2d_i size);

        bool IsValid() const {
            return cnt <= g_surface_limit;
        }
    };

    s_str_draw_info GenStrDrawInfo(const char* const str, const int font_index, const s_fonts& fonts, const s_vec_2d pos, const e_str_hor_align hor_align, const e_str_ver_align ver_align);
    s_rect GenStrCollider(const s_str_draw_info& draw_info);

    bool ExecDrawInstrs(const s_array<const a_draw_instr> instrs, const s_vec_2d_i window_size, const s_pers_render_data& pers_render_data, const s_surfaces& surfs, s_mem_arena& scratch_space);
    bool AppendDrawTextureInstr(s_list<a_draw_instr>& instrs, const int tex_index, const s_textures& textures, const s_rect_i src_rect, const s_vec_2d pos, const s_vec_2d origin = {0.5f, 0.5f}, const s_vec_2d scale = {1.0f, 1.0f}, const float rot = 0.0f, const s_vec_4d blend = {1.0f, 1.0f, 1.0f, 1.0f});
    bool AppendDrawTextureInstrsForStr(s_list<a_draw_instr>& instrs, const char* const str, const int font_index, const s_fonts& fonts, const s_vec_2d pos, const s_vec_4d blend, const e_str_hor_align hor_align, const e_str_ver_align ver_align);

    inline bool IsColorValid(const zf4::s_vec_3d col) {
        return col.x >= 0.0f && col.x <= 1.0f
            && col.y >= 0.0f && col.y <= 1.0f
            && col.z >= 0.0f && col.z <= 1.0f;
    }

    inline bool IsColorValid(const zf4::s_vec_4d col) {
        return col.x >= 0.0f && col.x <= 1.0f
            && col.y >= 0.0f && col.y <= 1.0f
            && col.z >= 0.0f && col.z <= 1.0f
            && col.w >= 0.0f && col.w <= 1.0f;
    }

    inline bool AppendDrawTextureInstrsForRect(s_list<a_draw_instr>& instrs, const s_rect rect, const s_vec_4d blend, const int px_tex_index, const s_textures& textures, const s_rect_i px_src_rect) {
        return AppendDrawTextureInstr(instrs, px_tex_index, textures, px_src_rect, rect.TopLeft(), {}, rect.Size(), 0.0f, blend);
    }
}
