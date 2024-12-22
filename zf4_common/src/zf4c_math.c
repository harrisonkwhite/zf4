#include <zf4c_math.h>

void init_identity_matrix_4x4(ZF4Matrix4x4* const mat) {
    assert(zf4_is_zero(mat, sizeof(*mat)));

    mat->elems[0][0] = 1.0f;
    mat->elems[1][1] = 1.0f;
    mat->elems[2][2] = 1.0f;
    mat->elems[3][3] = 1.0f;
}

void init_ortho_matrix_4x4(ZF4Matrix4x4* const mat, const float left, const float right, const float bottom, const float top, const float near, const float far) {
    assert(zf4_is_zero(mat, sizeof(*mat)));

    mat->elems[0][0] = 2.0f / (right - left);
    mat->elems[1][1] = 2.0f / (top - bottom);
    mat->elems[2][2] = -2.0f / (far - near);
    mat->elems[3][0] = -(right + left) / (right - left);
    mat->elems[3][1] = -(top + bottom) / (top - bottom);
    mat->elems[3][2] = -(far + near) / (far - near);
    mat->elems[3][3] = 1.0f;
}
