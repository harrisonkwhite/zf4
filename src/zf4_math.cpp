#include <zf4_math.h>

#include <limits>

namespace zf4 {
    static s_range_f ProjectPts(const s_array<const s_vec_2d> pts, const s_vec_2d edge) {
        s_range_f range = {std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest()};

        for (int i = 0; i < pts.len; ++i) {
            const float dot = Dot(pts[i], edge);

            if (dot < range.min) {
                range.min = dot;
            }

            if (dot > range.max) {
                range.max = dot;
            }
        }

        return range;
    }

    static bool CheckPolySeparation(const s_poly_view poly, const s_poly_view other) {
        for (int i = 0; i < poly.pts.len; ++i) {
            const s_vec_2d a = poly.pts[i];
            const s_vec_2d b = poly.pts[(i + 1) % poly.pts.len];

            const s_vec_2d normal = {(b.y - a.y), -(b.x - a.x)};

            const s_range_f poly_a_range = ProjectPts(poly.pts, normal);
            const s_range_f poly_b_range = ProjectPts(other.pts, normal);

            if (poly_a_range.max <= poly_b_range.min || poly_b_range.max <= poly_a_range.min) {
                return false;
            }
        }

        return true;
    }

    bool DoPolysIntersect(const s_poly_view poly_a, const s_poly_view poly_b) {
        return CheckPolySeparation(poly_a, poly_b) && CheckPolySeparation(poly_b, poly_a);
    }

    s_poly PushQuadPoly(const s_vec_2d pos, const s_vec_2d size, const s_vec_2d origin, s_mem_arena& mem_arena) {
        const s_poly poly = {
            .pts = PushArray<s_vec_2d>(4, mem_arena)
        };

        if (!IsStructZero(poly.pts)) {
            const zf4::s_vec_2d pos_base = pos - zf4::s_vec_2d(size.x * origin.x, size.y * origin.y);
            poly.pts[0] = pos_base;
            poly.pts[1] = pos_base + zf4::s_vec_2d(size.x, 0.0f);
            poly.pts[2] = pos_base + zf4::s_vec_2d(size.x, size.y);
            poly.pts[3] = pos_base + zf4::s_vec_2d(0.0f, size.y);
        }

        return poly;
    }

    s_poly PushRotatedQuadPoly(const s_vec_2d pos, const s_vec_2d size, const s_vec_2d origin, const float rot, s_mem_arena& mem_arena) {
        const s_poly poly = {
            .pts = PushArray<s_vec_2d>(4, mem_arena)
        };

        if (!IsStructZero(poly.pts)) {
            const zf4::s_vec_2d left_offs = LenDir(size.x * origin.x, rot + g_pi);
            const zf4::s_vec_2d up_offs = LenDir(size.y * origin.y, rot + (g_pi / 2.0f));
            const zf4::s_vec_2d right_offs = LenDir(size.x * (1.0f - origin.x), rot);
            const zf4::s_vec_2d down_offs = LenDir(size.y * (1.0f - origin.y), rot - (g_pi / 2.0f));

            poly.pts[0] = pos + left_offs + up_offs;
            poly.pts[1] = pos + right_offs + up_offs;
            poly.pts[2] = pos + right_offs + down_offs;
            poly.pts[3] = pos + left_offs + down_offs;
        }

        return poly;
    }

    s_poly PushPolyOfRectTranslation(const s_vec_2d rect_size, const s_vec_2d rect_pos_a, const s_vec_2d rect_pos_b, s_mem_arena& mem_arena) {
        if (rect_pos_a == rect_pos_b) {
            return PushQuadPoly(rect_pos_a, rect_size, {}, mem_arena);
        }

        if (rect_pos_a.x == rect_pos_b.x) {
            const s_poly poly = {
                .pts = PushArray<s_vec_2d>(4, mem_arena)
            };

            const s_vec_2d lower_rect_pos = rect_pos_a.y < rect_pos_b.y ? rect_pos_a : rect_pos_b;
            const s_vec_2d higher_rect_pos = rect_pos_a.y > rect_pos_b.y ? rect_pos_a : rect_pos_b;

            poly.pts[0] = lower_rect_pos;
            poly.pts[1] = {lower_rect_pos.x + rect_size.x, lower_rect_pos.y};
            poly.pts[2] = {lower_rect_pos.x + rect_size.x, higher_rect_pos.y + rect_size.y};
            poly.pts[3] = {lower_rect_pos.x, higher_rect_pos.y + rect_size.y};

            return poly;
        }

        if (rect_pos_a.y == rect_pos_b.y) {
            const s_poly poly = {
                .pts = PushArray<s_vec_2d>(4, mem_arena)
            };

            const s_vec_2d leftmost_rect_pos = rect_pos_a.y < rect_pos_b.y ? rect_pos_a : rect_pos_b;
            const s_vec_2d rightmost_rect_pos = rect_pos_a.y > rect_pos_b.y ? rect_pos_a : rect_pos_b;

            poly.pts[0] = leftmost_rect_pos;
            poly.pts[1] = {rightmost_rect_pos.x + rect_size.x, leftmost_rect_pos.y};
            poly.pts[2] = {rightmost_rect_pos.x + rect_size.x, leftmost_rect_pos.y + rect_size.y};
            poly.pts[3] = {leftmost_rect_pos.x, leftmost_rect_pos.y + rect_size.y};

            return poly;
        }

        const s_poly poly = {
            .pts = PushArray<s_vec_2d>(6, mem_arena)
        };

        const s_vec_2d leftmost_rect_pos = rect_pos_a.y < rect_pos_b.y ? rect_pos_a : rect_pos_b;
        const s_vec_2d rightmost_rect_pos = rect_pos_a.y > rect_pos_b.y ? rect_pos_a : rect_pos_b;

        if (leftmost_rect_pos.y < rightmost_rect_pos.y) {
            poly.pts[0] = leftmost_rect_pos;
            poly.pts[1] = {leftmost_rect_pos.x + rect_size.x, leftmost_rect_pos.y};
            poly.pts[2] = {rightmost_rect_pos.x + rect_size.x, rightmost_rect_pos.y};
            poly.pts[3] = {rightmost_rect_pos.x + rect_size.x, rightmost_rect_pos.y + rect_size.y};
            poly.pts[4] = {rightmost_rect_pos.x, rightmost_rect_pos.y + rect_size.y};
            poly.pts[5] = {leftmost_rect_pos.x, leftmost_rect_pos.y + rect_size.y};
        } else {
            poly.pts[0] = leftmost_rect_pos;
            poly.pts[1] = rightmost_rect_pos;
            poly.pts[2] = {rightmost_rect_pos.x + rect_size.x, rightmost_rect_pos.y};
            poly.pts[3] = {rightmost_rect_pos.x + rect_size.x, rightmost_rect_pos.y + rect_size.y};
            poly.pts[4] = {leftmost_rect_pos.x + rect_size.x, leftmost_rect_pos.y + rect_size.y};
            poly.pts[5] = {leftmost_rect_pos.x, leftmost_rect_pos.y + rect_size.y};
        }

        return poly;
    }

    bool DoesPolyIntersectWithRect(const s_poly_view poly, const s_rect rect) {
        const s_static_array<s_vec_2d, 4> rect_poly_pts = {
            zf4::s_vec_2d(rect.x, rect.y),
            zf4::s_vec_2d(rect.x + rect.width, rect.y),
            zf4::s_vec_2d(rect.x + rect.width, rect.y + rect.height),
            zf4::s_vec_2d(rect.x, rect.y + rect.height)
        };

        const s_poly_view rect_poly = {
            .pts = rect_poly_pts
        };

        return DoPolysIntersect(poly, rect_poly);
    }

    float PolyLeft(const s_poly_view poly) {
        assert(poly.pts.len > 0);

        float leftmost_x = poly.pts[0].x;

        for (int i = 1; i < poly.pts.len; ++i) {
            const float pt_x = poly.pts[i].x;

            if (pt_x < leftmost_x) {
                leftmost_x = pt_x;
            }
        }

        return leftmost_x;
    }

    float PolyRight(const s_poly_view poly) {
        assert(poly.pts.len > 0);

        float rightmost_x = poly.pts[0].x;

        for (int i = 1; i < poly.pts.len; ++i) {
            const float pt_x = poly.pts[i].x;

            if (pt_x > rightmost_x) {
                rightmost_x = pt_x;
            }
        }

        return rightmost_x;
    }

    float PolyTop(const s_poly_view poly) {
        assert(poly.pts.len > 0);

        float topmost_y = poly.pts[0].y;

        for (int i = 1; i < poly.pts.len; ++i) {
            const float pt_y = poly.pts[i].y;

            if (pt_y < topmost_y) {
                topmost_y = pt_y;
            }
        }

        return topmost_y;
    }

    float PolyBottom(const s_poly_view poly) {
        assert(poly.pts.len > 0);

        float bottommost_y = poly.pts[0].y;

        for (int i = 1; i < poly.pts.len; ++i) {
            const float pt_y = poly.pts[i].y;

            if (pt_y > bottommost_y) {
                bottommost_y = pt_y;
            }
        }

        return bottommost_y;
    }

    bool IsPolyQuad(const s_poly_view poly) {
        if (poly.pts.len != 4) {
            return false;
        }

        // Verify that all the angles are 90 degrees.
        for (int i = 0; i < poly.pts.len; ++i) {
            const zf4::s_vec_2d pt = poly.pts[i];
            const zf4::s_vec_2d pt_prev = poly.pts[WrappedIndex(i - 1, poly.pts.len)];
            const zf4::s_vec_2d pt_next = poly.pts[WrappedIndex(i + 1, poly.pts.len)];

            if (Dot(pt_prev - pt, pt_next - pt) != 0.0f) {
                return false;
            }
        }

        return true;
    }

    bool IsPolyQuadNonRot(const s_poly_view poly) {
        if (poly.pts.len != 4) {
            return false;
        }

        if (poly.pts[0].x == poly.pts[3].x && poly.pts[1].x == poly.pts[2].x
            && poly.pts[0].y == poly.pts[1].y && poly.pts[2].y == poly.pts[3].y) {
            return true;
        }

        if (poly.pts[0].x == poly.pts[1].x && poly.pts[2].x == poly.pts[3].x
            && poly.pts[0].y == poly.pts[3].y && poly.pts[1].y == poly.pts[2].y) {
            return true;
        }

        return false;
    }

    s_matrix_4x4 GenIdentityMatrix4x4() {
        s_matrix_4x4 mat = {};
        mat.elems[0][0] = 1.0f;
        mat.elems[1][1] = 1.0f;
        mat.elems[2][2] = 1.0f;
        mat.elems[3][3] = 1.0f;
        return mat;
    }

    s_matrix_4x4 GenOrthoMatrix4x4(const float left, const float right, const float bottom, const float top, const float near, const float far) {
        s_matrix_4x4 mat = {};
        mat.elems[0][0] = 2.0f / (right - left);
        mat.elems[1][1] = 2.0f / (top - bottom);
        mat.elems[2][2] = -2.0f / (far - near);
        mat.elems[3][0] = -(right + left) / (right - left);
        mat.elems[3][1] = -(top + bottom) / (top - bottom);
        mat.elems[3][2] = -(far + near) / (far - near);
        mat.elems[3][3] = 1.0f;
        return mat;
    }
}
