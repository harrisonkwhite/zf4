#ifndef ZF4C_MATH_H
#define ZF4C_MATH_H

#include <math.h>
#include <stdbool.h>

#define ZF4_MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define ZF4_MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#define ZF4_CLAMP(X, MIN, MAX) ZF4_MIN(ZF4_MAX(X, MIN), MAX)

typedef struct {
    float x, y;
} ZF4Vec2D;

typedef union {
    struct {
        float x, y, z;
    };

    struct {
        float r, g, b;
    };
} ZF4Vec3D;

typedef union {
    struct {
        float x, y, z, w;
    };

    struct {
        float r, g, b, a;
    };
} ZF4Vec4D;

typedef struct {
    int x, y;
} ZF4Pt2D;

typedef struct {
    int x, y;
    int width, height;
} ZF4Rect;

typedef struct {
    float x, y;
    float width, height;
} ZF4RectF;

typedef struct {
    float elems[4][4];
} ZF4Matrix4x4;

void zf4_init_identity_matrix_4x4(ZF4Matrix4x4* const mat);
void zf4_init_ortho_matrix_4x4(ZF4Matrix4x4* const mat, const float left, const float right, const float bottom, const float top, const float near, const float far);

inline float zf4_lerp(const float a, const float b, const float t) {
    return a + ((b - a) * t);
}

inline ZF4Vec2D zf4_create_vec_2d(const float x, const float y) {
    const ZF4Vec2D vec = {x, y};
    return vec;
}

inline ZF4Vec2D zf4_get_vec_2d_normalized(const ZF4Vec2D vec) {
    const float mag = sqrtf(powf(vec.x, 2) + powf(vec.y, 2));
    return zf4_create_vec_2d(vec.x / mag, vec.y / mag);
}

inline float zf4_calc_dist(const ZF4Vec2D a, const ZF4Vec2D b) {
    return sqrtf(powf(b.x - a.x, 2) + powf(b.y - a.y, 2));
}

inline float zf4_calc_dir(const ZF4Vec2D a, const ZF4Vec2D b) {
    return atan2f(b.y - a.y, b.x - a.x);
}

inline ZF4Vec2D zf4_calc_dir_vec_2d(const ZF4Vec2D a, const ZF4Vec2D b) {
    const ZF4Vec2D diff = {b.x - a.x, b.y - a.y};
    return zf4_get_vec_2d_normalized(diff);
}

inline ZF4Vec3D zf4_create_vec_3d(const float x, const float y, const float z) {
    const ZF4Vec3D vec = {x, y, z};
    return vec;
}

inline ZF4Vec4D zf4_create_vec_4d(const float x, const float y, const float z, const float w) {
    const ZF4Vec4D vec = {x, y, z, w};
    return vec;
}

inline ZF4Pt2D zf4_create_pt_2d(const int x, const int y) {
    const ZF4Pt2D pt = {x, y};
    return pt;
}

inline ZF4Rect zf4_create_rect(const int x, const int y, const int width, const int height) {
    const ZF4Rect rect = {x, y, width, height};
    return rect;
}

inline ZF4RectF zf4_create_rect_f(const float x, const float y, const float width, const float height) {
    const ZF4RectF rect = {x, y, width, height};
    return rect;
}

inline int zf4_get_rect_right(const ZF4Rect* const rect) {
    return rect->x + rect->width;
}

inline float zf4_get_rect_f_right(const ZF4RectF* const rect) {
    return rect->x + rect->width;
}

inline int zf4_get_rect_bottom(const ZF4Rect* const rect) {
    return rect->y + rect->height;
}

inline float zf4_get_rect_f_bottom(const ZF4RectF* const rect) {
    return rect->y + rect->height;
}

inline bool zf4_do_rects_intersect(const ZF4Rect* const a, const ZF4Rect* const b) {
    return a->x < zf4_get_rect_right(b) && zf4_get_rect_right(a) > b->x && a->y < zf4_get_rect_bottom(b) && zf4_get_rect_bottom(a) > b->y;
}

inline bool zf4_do_rect_fs_intersect(const ZF4RectF* const a, const ZF4RectF* const b) {
    return a->x < zf4_get_rect_f_right(b) && zf4_get_rect_f_right(a) > b->x && a->y < zf4_get_rect_f_bottom(b) && zf4_get_rect_f_bottom(a) > b->y;
}

#endif
