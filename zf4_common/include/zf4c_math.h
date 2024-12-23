#ifndef ZF4C_MATH_H
#define ZF4C_MATH_H

#define ZF4_MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define ZF4_MAX(X, Y) ((X) > (Y) ? (X) : (Y))

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
    float elems[4][4];
} ZF4Matrix4x4;

void zf4_init_identity_matrix_4x4(ZF4Matrix4x4* const mat);
void zf4_init_ortho_matrix_4x4(ZF4Matrix4x4* const mat, const float left, const float right, const float bottom, const float top, const float near, const float far);

inline float zf4_lerp(const float a, const float b, const float t) {
    return a + ((b - a) * t);
}

inline int zf4_get_rect_right(const ZF4Rect* const rect) {
    return rect->x + rect->width;
}

inline int zf4_get_rect_bottom(const ZF4Rect* const rect) {
    return rect->y + rect->height;
}

#endif
