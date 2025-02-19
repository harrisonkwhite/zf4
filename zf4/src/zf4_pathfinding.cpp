#include <zf4_pathfinding.h>

namespace zf4::pf {
    struct s_node_costs {
        float g_cost;
        float h_cost;
        float f_cost;
    };

    struct s_most_valuable_open_node_info {
        int index;
        int index_in_open_node_indexes_array;
    };

    static s_most_valuable_open_node_info FindMostValuableOpenNode(const zf4::s_array<const int> open_node_indexes, const zf4::s_array<const s_node_costs> node_costs) {
        assert(open_node_indexes.len > 0);

        s_most_valuable_open_node_info most_valuable_open_node_info = {
            .index = open_node_indexes[0],
            .index_in_open_node_indexes_array = 0
        };

        float most_valuable_node_f_cost = node_costs[most_valuable_open_node_info.index].f_cost;
        float most_valuable_node_h_cost = node_costs[most_valuable_open_node_info.index].h_cost;

        for (int i = 0; i < open_node_indexes.len; ++i) {
            const int open_node_index = open_node_indexes[i];
            const s_node_costs costs = node_costs[open_node_index];

            if (costs.f_cost < most_valuable_node_f_cost
                || (costs.f_cost == most_valuable_node_f_cost && costs.h_cost < most_valuable_node_h_cost)) {
                most_valuable_open_node_info = {
                    .index = open_node_index,
                    .index_in_open_node_indexes_array = i
                };

                most_valuable_node_f_cost = costs.f_cost;
                most_valuable_node_h_cost = costs.h_cost;
            }
        }

        return most_valuable_open_node_info;
    }

    static inline int FCost(const s_node_costs costs) {
        return costs.g_cost + costs.h_cost;
    }

    static int PathLen(const int dest_node_index, const zf4::s_array<const int> node_connection_indexes) {
        int node_index = dest_node_index;
        int len = 1; // Include the destination cell.

        while (node_connection_indexes[node_index] != -1) {
            node_index = node_connection_indexes[node_index];
            ++len;
        }

        return len;
    }

    e_path_gen_result GenPath(zf4::s_list<int> path_node_indexes, const int src_node_index, const int dest_node_index, const zf4::s_rect collider_base, const s_context& context, s_mem_arena& scratch_space) {
        auto open_node_indexes = zf4::PushList<int>(context.nodes.len, scratch_space);

        const auto node_states = zf4::PushArray<e_node_state>(context.nodes.len, scratch_space);
        const auto node_costs = zf4::PushArray<s_node_costs>(context.nodes.len, scratch_space);
        const auto node_connection_indexes = zf4::PushArray<int>(context.nodes.len, scratch_space);

        if (zf4::IsStructZero(node_states) || zf4::IsStructZero(node_costs) || zf4::IsStructZero(node_connection_indexes)) {
            return ek_path_gen_result_error;
        }

        memset(node_connection_indexes.elems_raw, -1, sizeof(*node_connection_indexes.elems_raw) * node_connection_indexes.len);

        zf4::ListAppend(open_node_indexes, src_node_index);
        node_costs[src_node_index].h_cost = zf4::Dist(context.nodes[src_node_index].pos, context.nodes[dest_node_index].pos);
        node_costs[src_node_index].f_cost = node_costs[src_node_index].h_cost;

        while (true) {
            if (open_node_indexes.len == 0) {
                // No open nodes exist, so a path cannot be found.
                return ek_path_gen_result_no_path;
            }

            const s_most_valuable_open_node_info most_valuable_open_node_info = FindMostValuableOpenNode({open_node_indexes.elems_raw, open_node_indexes.len}, node_costs);
            const s_node& mv_node = context.nodes[most_valuable_open_node_info.index];
            e_node_state& mv_node_state = node_states[most_valuable_open_node_info.index];
            const s_node_costs& mv_node_costs = node_costs[most_valuable_open_node_info.index];

            // Check if we've reached the destination.
            if (most_valuable_open_node_info.index == dest_node_index) {
                const int path_len = PathLen(dest_node_index, node_connection_indexes);
                path_node_indexes.len = zf4::Min(path_len, path_node_indexes.cap);

                int path_node_indexes_index = path_len - 1;
                int path_node_index = dest_node_index;

                while (path_node_indexes_index >= 0) {
                    if (path_node_indexes_index < node_connection_indexes.len) {
                        path_node_indexes[path_node_indexes_index] = path_node_index;
                    }

                    path_node_index = node_connection_indexes[path_node_index];
                    --path_node_indexes_index;
                }

                assert(path_node_indexes[0] == src_node_index); // Sanity check.

                return ek_path_gen_result_success;
            }

            // Remove the index of the most valuable open node from the list.
            open_node_indexes[most_valuable_open_node_info.index_in_open_node_indexes_array] = zf4::ListEnd(open_node_indexes);
            --open_node_indexes.len;

            // The node is now closed.
            mv_node_state = ek_node_state_closed;

            // Open neighbour nodes.
            for (int i = 0; i < mv_node.neighbour_indexes.len; ++i) {
                const int mv_node_neighbour_index = mv_node.neighbour_indexes[i];

                if (node_states[mv_node_neighbour_index] == ek_node_state_closed) {
                    continue;
                }

                const zf4::s_vec_2d mv_node_neighbour_pos = context.nodes[mv_node_neighbour_index].pos;

                const float prospective_g_cost = mv_node_costs.g_cost + zf4::Dist(mv_node.pos, mv_node_neighbour_pos);

                if (node_states[mv_node_neighbour_index] == ek_node_state_unexplored || prospective_g_cost < node_costs[mv_node_neighbour_index].g_cost) {
                    node_costs[mv_node_neighbour_index].g_cost = prospective_g_cost;
                    node_connection_indexes[mv_node_neighbour_index] = most_valuable_open_node_info.index;

                    if (node_states[mv_node_neighbour_index] == ek_node_state_unexplored) {
                        node_states[mv_node_neighbour_index] = ek_node_state_open;
                        zf4::ListAppend(open_node_indexes, mv_node_neighbour_index);

                        node_costs[mv_node_neighbour_index].h_cost = zf4::Dist(mv_node_neighbour_pos, context.nodes[dest_node_index].pos);
                    }

                    node_costs[mv_node_neighbour_index].f_cost = FCost(node_costs[mv_node_neighbour_index]);
                }
            }
        }
    }
}
