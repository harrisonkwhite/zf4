#pragma once

#include <zf4c_math.h>
#include <zf4c_mem.h>

namespace zf4::pf {
    enum e_node_state {
        ek_node_state_unexplored,
        ek_node_state_open,
        ek_node_state_closed
    };

    struct s_node {
        zf4::s_vec_2d pos;
        zf4::s_array<const int> neighbour_indexes;
    };

    struct s_context {
        zf4::s_array<const s_node> nodes;
        zf4::s_array<const int> node_neighbour_indexes;
        zf4::s_array<const zf4::s_rect> other_colliders;
    };

    enum e_path_gen_result {
        ek_path_gen_result_success,
        ek_path_gen_result_no_path,
        ek_path_gen_result_error
    };

    e_path_gen_result GenPath(zf4::s_list<int> path_node_indexes, const int src_node_index, const int dest_node_index, const zf4::s_rect collider_base, const s_context& context, s_mem_arena& scratch_space);
}
