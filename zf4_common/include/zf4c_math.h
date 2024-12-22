#ifndef ZF4C_MATH_H
#define ZF4C_MATH_H

#include <assert.h>
#include <zf4c_mem.h>

#define ZF4_MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define ZF4_MAX(X, Y) ((X) > (Y) ? (X) : (Y))

typedef struct {
    float x, y;
} ZF4Vec2D;

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

void init_identity_matrix_4x4(ZF4Matrix4x4* const mat);
void init_ortho_matrix_4x4(ZF4Matrix4x4* const mat, const float left, const float right, const float bottom, const float top, const float near, const float far);

#endif
