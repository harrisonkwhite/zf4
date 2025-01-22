#ifndef ZF4C_MATH_H
#define ZF4C_MATH_H

#include <math.h>
#include <assert.h>
#include <stdbool.h>

#define ZF4_MIN(A, B) ((A) < (B) ? (A) : (B))
#define ZF4_MAX(A, B) ((A) > (B) ? (A) : (B))

#define ZF4_PI 3.14159265359f

#define V2(X, Y) (s_vec_2d) { X, Y }
#define V2_ZERO V2(0.0f, 0.0f)
#define V2_ONE V2(1.0f, 1.0f)

#define V3(X, Y, Z) (s_vec_3d) { X, Y, Z }
#define V4(X, Y, Z, W) (s_vec_4d) { X, Y, Z, W }

typedef struct {
    float x;
    float y;
} s_vec_2d;

typedef struct {
    int x;
    int y;
} s_vec_2d_i;

typedef struct {
    float x;
    float y;
    float z;
} s_vec_3d;

typedef struct {
    float x;
    float y;
    float z;
    float w;
} s_vec_4d;

typedef struct {
    float x;
    float y;
    float width;
    float height;
} s_rect;

typedef struct {
    int x;
    int y;
    int width;
    int height;
} s_rect_i;

typedef struct {
    float left;
    float top;
    float right;
    float bottom;
} s_rect_edges;

typedef struct {
    int left;
    int top;
    int right;
    int bottom;
} s_rect_edges_i;

typedef struct {
    float elems[4][4];
} s_matrix_4x4;

void InitIdentityMatrix4x4(s_matrix_4x4* const mat);
void InitOrthoMatrix4x4(s_matrix_4x4* const mat, const float left, const float right, const float bottom, const float top, const float near, const float far);

inline s_vec_2d V2Sum(const s_vec_2d a, const s_vec_2d b) {
    return V2(a.x + b.x, a.y + b.y);
}

inline s_vec_2d V2Diff(const s_vec_2d a, const s_vec_2d b) {
    return V2(a.x - b.x, a.y - b.y);
}

inline s_vec_2d V2Scaled(const s_vec_2d vec, const float scalar) {
    return V2(vec.x * scalar, vec.y * scalar);
}

inline float V2Mag(const s_vec_2d vec) {
    return sqrtf((vec.x * vec.x) + (vec.y * vec.y));
}

inline s_vec_2d V2Normalized(const s_vec_2d vec) {
    const float mag = V2Mag(vec);
    assert(mag != 0.0f);
    return V2Scaled(vec, 1.0f / mag);
}

inline bool AreV2sEqual(const s_vec_2d a, const s_vec_2d b) {
    return a.x == b.x && a.y == b.y;
}

inline float Dist(const s_vec_2d a, const s_vec_2d b) {
    return V2Mag(V2Diff(b, a));
}

inline float Dir(const s_vec_2d a, const s_vec_2d b) {
    return atan2f(-(b.y - a.y), b.x - a.x);
}

inline s_vec_2d LenDir(const float len, const float dir) {
    return V2(len * cosf(dir), -len * sinf(dir));
}

inline s_vec_2d RectPos(const s_rect* const rect) {
    return V2(rect->x, rect->y);
}

inline s_vec_2d RectSize(const s_rect* const rect) {
    return V2(rect->width, rect->height);
}

inline s_vec_2d RectCenter(const s_rect* const rect) {
    return V2(
        rect->x + (rect->width / 2.0f),
        rect->y + (rect->height / 2.0f)
    );
}

inline float RectRight(const s_rect* const rect) {
    return rect->x + rect->width;
}

inline int RectRightI(const s_rect_i* const rect) {
    return rect->x + rect->width;
}

inline float RectBottom(const s_rect* const rect) {
    return rect->y + rect->height;
}

inline int RectBottomI(const s_rect_i* const rect) {
    return rect->y + rect->height;
}

inline float RectWidth(const s_rect_edges* const edges) {
    return edges->right - edges->left;
}

inline float RectHeight(const s_rect_edges* const edges) {
    return edges->bottom - edges->top;
}

inline bool DoRectsIntersect(const s_rect* const a, const s_rect* const b) {
    return a->x < RectRight(b)
        && RectRight(a) > b->x
        && a->y < RectBottom(b)
        && RectBottom(a) > b->y;
}

inline float Lerp(const float a, const float b, const float t) {
    return a + ((b - a) * t);
}

inline s_vec_2d LerpV2(const s_vec_2d a, const s_vec_2d b, const float t) {
    return V2(Lerp(a.x, b.x, t), Lerp(a.y, b.y, t));
}

#endif
