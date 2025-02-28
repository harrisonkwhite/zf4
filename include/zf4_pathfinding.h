#pragma once

#include <zf4_math.h>
#include <zf4_mem.h>

namespace zf4::pf {
    enum e_node_state {
        ek_node_state_unexplored,
        ek_node_state_open,
        ek_node_state_closed
    };

    struct s_node {
        s_vec_2d pos;
        s_array<const int> neighbour_indexes;
    };

    struct s_context {
        s_array<const s_node> nodes;
        s_array<const int> node_neighbour_indexes;
        s_array<const s_rect> other_colliders;
    };

    enum e_path_gen_result {
        ek_path_gen_result_success,
        ek_path_gen_result_no_path,
        ek_path_gen_result_error
    };

    s_context GenContextForGrid(const s_vec_2d node_spacing, const s_vec_2d_i grid_size, const s_array<const s_rect> other_colliders, s_mem_arena& mem_arena);
    e_path_gen_result GenPath(s_list<int>& path_node_indexes, const int src_node_index, const int dest_node_index, const s_rect collider_base, const s_context& context, s_mem_arena& scratch_space);
}
