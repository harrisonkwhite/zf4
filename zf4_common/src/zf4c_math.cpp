#include <zf4c_math.h>

namespace zf4 {
    bool DoesRectIntersectWithOtherRects(const s_rect rect, const s_array<s_rect> other_rects) {
        for (int i = 0; i < other_rects.len; ++i) {
            if (DoRectsIntersect(rect, other_rects[i])) {
                return true;
            }
        }

        return false;
    }

    void InitIdentityMatrix4x4(s_matrix_4x4& mat) {
        assert(IsStructZero(mat));

        mat.elems[0][0] = 1.0f;
        mat.elems[1][1] = 1.0f;
        mat.elems[2][2] = 1.0f;
        mat.elems[3][3] = 1.0f;
    }

    void InitOrthoMatrix4x4(s_matrix_4x4& mat, const float left, const float right, const float bottom, const float top, const float near, const float far) {
        assert(IsStructZero(mat));

        mat.elems[0][0] = 2.0f / (right - left);
        mat.elems[1][1] = 2.0f / (top - bottom);
        mat.elems[2][2] = -2.0f / (far - near);
        mat.elems[3][0] = -(right + left) / (right - left);
        mat.elems[3][1] = -(top + bottom) / (top - bottom);
        mat.elems[3][2] = -(far + near) / (far - near);
        mat.elems[3][3] = 1.0f;
    }
}
