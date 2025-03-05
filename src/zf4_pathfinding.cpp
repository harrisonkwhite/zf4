#include <zf4_pathfinding.h>

namespace zf4::pf {
#if 0
    struct s_node_costs {
        float g_cost;
        float h_cost;
        float f_cost;
    };

    struct s_most_valuable_open_node_info {
        int index;
        int index_in_open_node_indexes_array;
    };

    static s_most_valuable_open_node_info FindMostValuableOpenNode(const s_array<const int> open_node_indexes, const s_array<const s_node_costs> node_costs) {
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

    static int PathLen(const int dest_node_index, const s_array<const int> node_connection_indexes) {
        int node_index = dest_node_index;
        int len = 1; // Include the destination cell.

        while (node_connection_indexes[node_index] != -1) {
            node_index = node_connection_indexes[node_index];
            ++len;
        }

        return len;
    }

    s_context GenContextForGrid(const s_vec_2d node_spacing, const s_vec_2d_i grid_size, const s_array<const s_rect> other_colliders, s_mem_arena& mem_arena) {
        assert(node_spacing.x > 0.0f && node_spacing.y > 0.0f);
        assert(grid_size.x > 1 && grid_size.y > 1);

        const auto nodes = PushArray<s_node>(grid_size.x * grid_size.y, mem_arena);

        if (IsStructZero(nodes)) {
            return {};
        }

        enum e_grid_section {
            ek_grid_section_top_left_corner,
            ek_grid_section_top_right_corner,
            ek_grid_section_bottom_right_corner,
            ek_grid_section_bottom_left_corner,
            ek_grid_section_top_side_excluding_corner,
            ek_grid_section_right_side_excluding_corner,
            ek_grid_section_bottom_side_excluding_corner,
            ek_grid_section_left_side_excluding_corner,
            ek_grid_section_insides,

            eks_grid_section_cnt
        };

        const s_static_array<int, eks_grid_section_cnt> neighbour_index_cnt_per_node_in_grid_section = {
            3,
            3,
            3,
            3,
            5,
            5,
            5,
            5,
            8
        };

        const int neighbour_index_cnt_corners = neighbour_index_cnt_per_node_in_grid_section[ek_grid_section_top_left_corner] * 4;
        const int neighbour_index_cnt_sides_excluding_corners = neighbour_index_cnt_per_node_in_grid_section[ek_grid_section_top_side_excluding_corner] * ((grid_size.x - 2) * 4);
        const int neighbour_index_cnt_insides = neighbour_index_cnt_per_node_in_grid_section[ek_grid_section_insides] * ((grid_size.x - 2) * (grid_size.y - 2));
        const int neighbour_index_cnt = neighbour_index_cnt_corners + neighbour_index_cnt_sides_excluding_corners + neighbour_index_cnt_insides;

        const auto neighbour_indexes = PushArray<int>(neighbour_index_cnt, mem_arena);

        if (IsStructZero(neighbour_indexes)) {
            return {};
        }

        int neighbour_indexes_index = 0;

        for (int y = 0; y < grid_size.y; ++y) {
            for (int x = 0; x < grid_size.x; ++x) {
                const int index = ToIndex(x, y, grid_size.x);

                nodes[index].pos = {x * node_spacing.x, y * node_spacing.y};

                const auto grid_section = [x, y, grid_size]() -> e_grid_section {
                    if (x == 0 && y == 0) {
                        return ek_grid_section_top_left_corner;
                    }

                    if (x == grid_size.x - 1 && y == 0) {
                        return ek_grid_section_top_right_corner;
                    }

                    if (x == grid_size.x - 1 && y == grid_size.y - 1) {
                        return ek_grid_section_bottom_right_corner;
                    }

                    if (x == 0 && y == grid_size.y - 1) {
                        return ek_grid_section_bottom_left_corner;
                    }

                    if (y == 0 && x >= 1 && x < grid_size.x - 1) {
                        return ek_grid_section_top_side_excluding_corner;
                    }

                    if (x == grid_size.x - 1 && y >= 1 && y < grid_size.y - 1) {
                        return ek_grid_section_right_side_excluding_corner;
                    }

                    if (y == grid_size.y - 1 && x >= 1 && x < grid_size.x - 1) {
                        return ek_grid_section_bottom_side_excluding_corner;
                    }

                    if (x == 0 && y >= 1 && y < grid_size.y - 1) {
                        return ek_grid_section_left_side_excluding_corner;
                    }

                    return ek_grid_section_insides;
                }();

                nodes[index].neighbour_indexes = {
                    .elems_raw = neighbour_indexes.elems_raw + neighbour_indexes_index,
                    .len = neighbour_index_cnt_per_node_in_grid_section[grid_section]
                };

                switch (grid_section) {
                    case ek_grid_section_top_left_corner:
                        neighbour_indexes[neighbour_indexes_index + 0] = ToIndex(x + 1, y, grid_size.x);
                        neighbour_indexes[neighbour_indexes_index + 1] = ToIndex(x + 1, y + 1, grid_size.x);
                        neighbour_indexes[neighbour_indexes_index + 2] = ToIndex(x, y + 1, grid_size.x);
                        break;

                    case ek_grid_section_top_right_corner:
                        neighbour_indexes[neighbour_indexes_index + 0] = ToIndex(x, y + 1, grid_size.x);
                        neighbour_indexes[neighbour_indexes_index + 1] = ToIndex(x - 1, y + 1, grid_size.x);
                        neighbour_indexes[neighbour_indexes_index + 2] = ToIndex(x - 1, y, grid_size.x);
                        break;

                    case ek_grid_section_bottom_right_corner:
                        neighbour_indexes[neighbour_indexes_index + 0] = ToIndex(x - 1, y, grid_size.x);
                        neighbour_indexes[neighbour_indexes_index + 1] = ToIndex(x - 1, y - 1, grid_size.x);
                        neighbour_indexes[neighbour_indexes_index + 2] = ToIndex(x, y - 1, grid_size.x);
                        break;

                    case ek_grid_section_bottom_left_corner:
                        neighbour_indexes[neighbour_indexes_index + 0] = ToIndex(x, y - 1, grid_size.x);
                        neighbour_indexes[neighbour_indexes_index + 1] = ToIndex(x + 1, y - 1, grid_size.x);
                        neighbour_indexes[neighbour_indexes_index + 2] = ToIndex(x + 1, y, grid_size.x);
                        break;

                    case ek_grid_section_top_side_excluding_corner:
                        neighbour_indexes[neighbour_indexes_index + 0] = ToIndex(x + 1, y, grid_size.x);
                        neighbour_indexes[neighbour_indexes_index + 1] = ToIndex(x + 1, y + 1, grid_size.x);
                        neighbour_indexes[neighbour_indexes_index + 2] = ToIndex(x, y + 1, grid_size.x);
                        neighbour_indexes[neighbour_indexes_index + 3] = ToIndex(x - 1, y + 1, grid_size.x);
                        neighbour_indexes[neighbour_indexes_index + 4] = ToIndex(x - 1, y, grid_size.x);
                        break;

                    case ek_grid_section_right_side_excluding_corner:
                        neighbour_indexes[neighbour_indexes_index + 0] = ToIndex(x - 1, y - 1, grid_size.x);
                        neighbour_indexes[neighbour_indexes_index + 1] = ToIndex(x, y - 1, grid_size.x);
                        neighbour_indexes[neighbour_indexes_index + 2] = ToIndex(x, y + 1, grid_size.x);
                        neighbour_indexes[neighbour_indexes_index + 3] = ToIndex(x - 1, y + 1, grid_size.x);
                        neighbour_indexes[neighbour_indexes_index + 4] = ToIndex(x - 1, y, grid_size.x);
                        break;

                    case ek_grid_section_bottom_side_excluding_corner:
                        neighbour_indexes[neighbour_indexes_index + 0] = ToIndex(x - 1, y - 1, grid_size.x);
                        neighbour_indexes[neighbour_indexes_index + 1] = ToIndex(x, y - 1, grid_size.x);
                        neighbour_indexes[neighbour_indexes_index + 2] = ToIndex(x + 1, y - 1, grid_size.x);
                        neighbour_indexes[neighbour_indexes_index + 3] = ToIndex(x + 1, y, grid_size.x);
                        neighbour_indexes[neighbour_indexes_index + 4] = ToIndex(x - 1, y, grid_size.x);
                        break;

                    case ek_grid_section_left_side_excluding_corner:
                        neighbour_indexes[neighbour_indexes_index + 0] = ToIndex(x, y - 1, grid_size.x);
                        neighbour_indexes[neighbour_indexes_index + 1] = ToIndex(x + 1, y - 1, grid_size.x);
                        neighbour_indexes[neighbour_indexes_index + 2] = ToIndex(x + 1, y, grid_size.x);
                        neighbour_indexes[neighbour_indexes_index + 3] = ToIndex(x + 1, y + 1, grid_size.x);
                        neighbour_indexes[neighbour_indexes_index + 4] = ToIndex(x, y + 1, grid_size.x);
                        break;

                    case ek_grid_section_insides:
                        neighbour_indexes[neighbour_indexes_index + 0] = ToIndex(x - 1, y - 1, grid_size.x);
                        neighbour_indexes[neighbour_indexes_index + 1] = ToIndex(x, y - 1, grid_size.x);
                        neighbour_indexes[neighbour_indexes_index + 2] = ToIndex(x + 1, y - 1, grid_size.x);
                        neighbour_indexes[neighbour_indexes_index + 3] = ToIndex(x + 1, y, grid_size.x);
                        neighbour_indexes[neighbour_indexes_index + 4] = ToIndex(x + 1, y + 1, grid_size.x);
                        neighbour_indexes[neighbour_indexes_index + 5] = ToIndex(x, y + 1, grid_size.x);
                        neighbour_indexes[neighbour_indexes_index + 6] = ToIndex(x - 1, y + 1, grid_size.x);
                        neighbour_indexes[neighbour_indexes_index + 7] = ToIndex(x - 1, y, grid_size.x);
                        break;
                }

                neighbour_indexes_index += neighbour_index_cnt_per_node_in_grid_section[grid_section];
            }
        }

        return {
            .nodes = nodes,
            .node_neighbour_indexes = neighbour_indexes,
            .other_colliders = other_colliders
        };
    }

    e_path_gen_result GenPath(s_list<int>& path_node_indexes, const int src_node_index, const int dest_node_index, const s_rect collider_base, const s_context& context, s_mem_arena& scratch_space) {
        assert(src_node_index >= 0 && src_node_index < context.nodes.len);
        assert(dest_node_index >= 0 && dest_node_index < context.nodes.len);

        auto open_node_indexes = PushList<int>(context.nodes.len, scratch_space);

        const auto node_states = PushArray<e_node_state>(context.nodes.len, scratch_space);
        const auto node_costs = PushArray<s_node_costs>(context.nodes.len, scratch_space);
        const auto node_connection_indexes = PushArray<int>(context.nodes.len, scratch_space);

        if (IsStructZero(node_states) || IsStructZero(node_costs) || IsStructZero(node_connection_indexes)) {
            return ek_path_gen_result_error;
        }

        memset(node_connection_indexes.elems_raw, -1, sizeof(*node_connection_indexes.elems_raw) * node_connection_indexes.len);

        ListAppend(open_node_indexes, src_node_index);
        node_costs[src_node_index].h_cost = Dist(context.nodes[src_node_index].pos, context.nodes[dest_node_index].pos);
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
                path_node_indexes.len = Min(path_len, path_node_indexes.cap);

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
            open_node_indexes[most_valuable_open_node_info.index_in_open_node_indexes_array] = ListEnd(open_node_indexes);
            --open_node_indexes.len;

            // The node is now closed.
            mv_node_state = ek_node_state_closed;

            // Open neighbour nodes.
            for (int i = 0; i < mv_node.neighbour_indexes.len; ++i) {
                const int mv_node_neighbour_index = mv_node.neighbour_indexes[i];

                if (node_states[mv_node_neighbour_index] == ek_node_state_closed) {
                    continue;
                }

                const s_vec_2d mv_node_neighbour_pos = context.nodes[mv_node_neighbour_index].pos;

                const float prospective_g_cost = mv_node_costs.g_cost + Dist(mv_node.pos, mv_node_neighbour_pos);

                if (node_states[mv_node_neighbour_index] == ek_node_state_unexplored || prospective_g_cost < node_costs[mv_node_neighbour_index].g_cost) {
                    node_costs[mv_node_neighbour_index].g_cost = prospective_g_cost;
                    node_connection_indexes[mv_node_neighbour_index] = most_valuable_open_node_info.index;

                    if (node_states[mv_node_neighbour_index] == ek_node_state_unexplored) {
                        node_states[mv_node_neighbour_index] = ek_node_state_open;
                        ListAppend(open_node_indexes, mv_node_neighbour_index);

                        node_costs[mv_node_neighbour_index].h_cost = Dist(mv_node_neighbour_pos, context.nodes[dest_node_index].pos);
                    }

                    node_costs[mv_node_neighbour_index].f_cost = FCost(node_costs[mv_node_neighbour_index]);
                }
            }
        }
    }
#endif
}
