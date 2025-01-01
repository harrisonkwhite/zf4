#pragma once

#include <cmath>
#include <cstdbool>

namespace zf4 {
    constexpr float gk_pi = 3.14159265359f;

    struct Vec2D {
        float x, y;
    };

    typedef union {
        struct {
            float x, y, z;
        };

        struct {
            float r, g, b;
        };
    } Vec3D;

    typedef union {
        struct {
            float x, y, z, w;
        };

        struct {
            float r, g, b, a;
        };
    } Vec4D;

    typedef struct {
        int x, y;
    } Pt2D;

    typedef struct {
        int x, y;
        int width, height;
    } Rect;

    typedef struct {
        float x, y;
        float width, height;
    } RectF;

    typedef struct {
        float elems[4][4];
    } Matrix4x4;

    void init_identity_matrix_4x4(Matrix4x4* const mat);
    void init_ortho_matrix_4x4(Matrix4x4* const mat, const float left, const float right, const float bottom, const float top, const float near, const float far);

    inline float lerp(const float a, const float b, const float t) {
        return a + ((b - a) * t);
    }

    inline Vec2D lerp_vec_2d(const Vec2D a, const Vec2D b, const float t) {
        return {lerp(a.x, b.x, t), lerp(a.y, b.y, t)};
    }

    inline Vec2D create_vec_2d(const float x, const float y) {
        const Vec2D vec = {x, y};
        return vec;
    }

    inline float calc_vec_2d_mag(const Vec2D vec) {
        return sqrtf((vec.x * vec.x) + (vec.y * vec.y));
    }

    inline Vec2D calc_vec_2d_normal(const Vec2D vec) {
        const float mag = calc_vec_2d_mag(vec);
        return create_vec_2d(vec.x / mag, vec.y / mag);
    }

    inline Vec2D calc_vec_2d_sum(const Vec2D a, const Vec2D b) {
        return create_vec_2d(a.x + b.x, a.y + b.y);
    }

    inline Vec2D calc_vec_2d_diff(const Vec2D a, const Vec2D b) {
        return create_vec_2d(a.x - b.x, a.y - b.y);
    }

    inline Vec2D calc_vec_2d_scaled(const Vec2D vec, const float scalar) {
        return create_vec_2d(vec.x * scalar, vec.y * scalar);
    }

    inline Vec2D calc_vec_2d_avg(const Vec2D a, const Vec2D b) {
        return create_vec_2d((a.x + b.x) / 2.0f, (a.y + b.y) / 2.0f);
    }

    inline float calc_vec_2d_dist(const Vec2D a, const Vec2D b) {
        const Vec2D diff = calc_vec_2d_diff(b, a);
        return calc_vec_2d_mag(diff);
    }

    inline Vec2D calc_vec_2d_dir(const Vec2D a, const Vec2D b) {
        const Vec2D diff = calc_vec_2d_diff(b, a);
        return calc_vec_2d_normal(diff);
    }

    inline float calc_vec_2d_dir_rads(const Vec2D a, const Vec2D b) {
        return atan2f(-(b.y - a.y), b.x - a.x);
    }

    inline Vec2D calc_len_dir_vec_2d(const float len, const float dir) {
        return calc_vec_2d_scaled(create_vec_2d(cosf(dir), -sinf(dir)), len);
    }

    inline Vec3D create_vec_3d(const float x, const float y, const float z) {
        const Vec3D vec = {x, y, z};
        return vec;
    }

    inline Vec4D create_vec_4d(const float x, const float y, const float z, const float w) {
        const Vec4D vec = {x, y, z, w};
        return vec;
    }

    inline Pt2D create_pt_2d(const int x, const int y) {
        const Pt2D pt = {x, y};
        return pt;
    }

    inline Rect create_rect(const int x, const int y, const int width, const int height) {
        const Rect rect = {x, y, width, height};
        return rect;
    }

    inline RectF create_rect_f(const float x, const float y, const float width, const float height) {
        const RectF rect = {x, y, width, height};
        return rect;
    }

    inline Pt2D get_rect_pos(const Rect* const rect) {
        return {rect->x, rect->y};
    }

    inline Vec2D get_rect_f_pos(const RectF* const rect) {
        return {rect->x, rect->y};
    }

    inline Pt2D get_rect_size(const Rect* const rect) {
        return {rect->width, rect->height};
    }

    inline Vec2D get_rect_f_size(const RectF* const rect) {
        return {rect->width, rect->height};
    }

    inline Pt2D get_rect_center(const Rect* const rect) {
        return {rect->x + (rect->width / 2), rect->y + (rect->height / 2)};
    }

    inline Vec2D get_rect_f_center(const RectF* const rect) {
        return {rect->x + (rect->width / 2.0f), rect->y + (rect->height / 2.0f)};
    }

    inline int get_rect_right(const Rect* const rect) {
        return rect->x + rect->width;
    }

    inline float get_rect_f_right(const RectF* const rect) {
        return rect->x + rect->width;
    }

    inline int get_rect_bottom(const Rect* const rect) {
        return rect->y + rect->height;
    }

    inline float get_rect_f_bottom(const RectF* const rect) {
        return rect->y + rect->height;
    }

    inline bool do_rects_intersect(const Rect* const a, const Rect* const b) {
        return a->x < get_rect_right(b) && get_rect_right(a) > b->x && a->y < get_rect_bottom(b) && get_rect_bottom(a) > b->y;
    }

    inline bool do_rect_fs_intersect(const RectF* const a, const RectF* const b) {
        return a->x < get_rect_f_right(b) && get_rect_f_right(a) > b->x && a->y < get_rect_f_bottom(b) && get_rect_f_bottom(a) > b->y;
    }
}
