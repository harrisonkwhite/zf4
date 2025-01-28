#pragma once

#include <cmath>
#include <cassert>
#include <zf4c_mem.h>

#define ZF4_MIN(A, B) ((A) < (B) ? (A) : (B))
#define ZF4_MAX(A, B) ((A) > (B) ? (A) : (B))

namespace zf4 {
    constexpr float g_pi = 3.14159265359f;

    struct s_vec_2d {
        float x;
        float y;

        s_vec_2d operator+(const s_vec_2d& other) const {
            return {x + other.x, y + other.y};
        }

        s_vec_2d operator-(const s_vec_2d& other) const {
            return {x - other.x, y - other.y};
        }

        s_vec_2d operator*(const float scalar) const {
            return {x * scalar, y * scalar};
        }

        s_vec_2d operator/(const float scalar) const {
            return {x / scalar, y / scalar};
        }

        s_vec_2d& operator+=(const s_vec_2d& other) {
            x += other.x;
            y += other.y;
            return *this;
        }

        s_vec_2d& operator-=(const s_vec_2d& other) {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        s_vec_2d& operator*=(const float scalar) {
            x *= scalar;
            y *= scalar;
            return *this;
        }

        s_vec_2d& operator/=(const float scalar) {
            x /= scalar;
            y /= scalar;
            return *this;
        }

        bool operator==(const s_vec_2d& other) const {
            return x == other.x && y == other.y;
        }

        bool operator!=(const s_vec_2d& other) const {
            return x != other.x || y != other.y;
        }
    };

    struct s_vec_2d_i {
        int x;
        int y;

        s_vec_2d_i operator+(const s_vec_2d_i& other) const {
            return {x + other.x, y + other.y};
        }

        s_vec_2d operator+(const s_vec_2d& other) const {
            return {x + other.x, y + other.y};
        }

        s_vec_2d_i operator-(const s_vec_2d_i& other) const {
            return {x - other.x, y - other.y};
        }

        s_vec_2d operator-(const s_vec_2d& other) const {
            return {x - other.x, y - other.y};
        }

        s_vec_2d_i operator*(const int scalar) const {
            return {x * scalar, y * scalar};
        }

        s_vec_2d operator*(const float scalar) const {
            return {x * scalar, y * scalar};
        }

        s_vec_2d_i operator/(const int scalar) const {
            return {x / scalar, y / scalar};
        }

        s_vec_2d operator/(const float scalar) const {
            return {x / scalar, y / scalar};
        }

        s_vec_2d_i& operator+=(const s_vec_2d_i& other) {
            x += other.x;
            y += other.y;
            return *this;
        }

        s_vec_2d_i& operator-=(const s_vec_2d_i& other) {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        s_vec_2d_i& operator*=(const int scalar) {
            x *= scalar;
            y *= scalar;
            return *this;
        }

        s_vec_2d_i& operator/=(const int scalar) {
            x /= scalar;
            y /= scalar;
            return *this;
        }

        bool operator==(const s_vec_2d_i& other) const {
            return x == other.x && y == other.y;
        }

        bool operator!=(const s_vec_2d_i& other) const {
            return x != other.x || y != other.y;
        }

        operator s_vec_2d() const {
            return {static_cast<float>(x), static_cast<float>(y)};
        }
    };

    struct s_vec_3d {
        float x;
        float y;
        float z;

        s_vec_3d operator+(const s_vec_3d& other) const {
            return {x + other.x, y + other.y, z + other.z};
        }

        s_vec_3d operator-(const s_vec_3d& other) const {
            return {x - other.x, y - other.y, z - other.z};
        }

        s_vec_3d operator*(const float scalar) const {
            return {x * scalar, y * scalar, z * scalar};
        }

        s_vec_3d operator/(const float scalar) const {
            return {x / scalar, y / scalar, z / scalar};
        }

        s_vec_3d& operator+=(const s_vec_3d& other) {
            x += other.x;
            y += other.y;
            z += other.z;
            return *this;
        }

        s_vec_3d& operator-=(const s_vec_3d& other) {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            return *this;
        }

        s_vec_3d& operator*=(const float scalar) {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            return *this;
        }

        s_vec_3d& operator/=(const float scalar) {
            x /= scalar;
            y /= scalar;
            z /= scalar;
            return *this;
        }

        bool operator==(const s_vec_3d& other) const {
            return x == other.x && y == other.y && z == other.z;
        }

        bool operator!=(const s_vec_3d& other) const {
            return x != other.x || y != other.y || z != other.z;
        }
    };

    struct s_vec_4d {
        float x;
        float y;
        float z;
        float w;

        s_vec_4d operator+(const s_vec_4d& other) const {
            return {x + other.x, y + other.y, z + other.z, w + other.w};
        }

        s_vec_4d operator-(const s_vec_4d& other) const {
            return {x - other.x, y - other.y, z - other.z, w - other.w};
        }

        s_vec_4d operator*(const float scalar) const {
            return {x * scalar, y * scalar, z * scalar, w * scalar};
        }

        s_vec_4d operator/(const float scalar) const {
            return {x / scalar, y / scalar, z / scalar, w / scalar};
        }

        s_vec_4d& operator+=(const s_vec_4d& other) {
            x += other.x;
            y += other.y;
            z += other.z;
            w += other.w;
            return *this;
        }

        s_vec_4d& operator-=(const s_vec_4d& other) {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            w -= other.w;
            return *this;
        }

        s_vec_4d& operator*=(const float scalar) {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            w *= scalar;
            return *this;
        }

        s_vec_4d& operator/=(const float scalar) {
            x /= scalar;
            y /= scalar;
            z /= scalar;
            w /= scalar;
            return *this;
        }

        bool operator==(const s_vec_4d& other) const {
            return x == other.x && y == other.y && z == other.z && w == other.w;
        }

        bool operator!=(const s_vec_4d& other) const {
            return x != other.x || y != other.y || z != other.z || w != other.w;
        }
    };

    struct s_rect {
        float x;
        float y;
        float width;
        float height;
    };

    struct s_rect_i {
        int x;
        int y;
        int width;
        int height;
    };

    struct s_rect_edges {
        float left;
        float top;
        float right;
        float bottom;
    };

    struct s_rect_edges_i {
        int left;
        int top;
        int right;
        int bottom;
    };

    struct s_matrix_4x4 {
        s_static_array<s_static_array<float, 4>, 4> elems;
    };

    void InitIdentityMatrix4x4(s_matrix_4x4& mat);
    void InitOrthoMatrix4x4(s_matrix_4x4& mat, const float left, const float right, const float bottom, const float top, const float near, const float far);

    inline float Magnitude(const s_vec_2d vec) {
        return sqrtf((vec.x * vec.x) + (vec.y * vec.y));
    }

    inline s_vec_2d Normal(const s_vec_2d vec) {
        const float mag = Magnitude(vec);
        assert(mag != 0.0f);
        return vec * (1.0f / mag);
    }

    inline float Dist(const s_vec_2d a, const s_vec_2d b) {
        return Magnitude(b - a);
    }

    inline float Dir(const s_vec_2d a, const s_vec_2d b) {
        return atan2f(-(b.y - a.y), b.x - a.x);
    }

    inline s_vec_2d Dir(const float dir) {
        return {cosf(dir), -sinf(dir)};
    }

    inline s_vec_2d LenDir(const float len, const float dir) {
        return {len * cosf(dir), -len * sinf(dir)};
    }

    inline s_vec_2d RectPos(const s_rect rect) {
        return {rect.x, rect.y};
    }

    inline s_vec_2d RectSize(const s_rect rect) {
        return {rect.width, rect.height};
    }

    inline s_vec_2d RectCenter(const s_rect rect) {
        return {
            rect.x + (rect.width / 2.0f),
            rect.y + (rect.height / 2.0f)
        };
    }

    inline float RectRight(const s_rect rect) {
        return rect.x + rect.width;
    }

    inline int RectRight(const s_rect_i rect) {
        return rect.x + rect.width;
    }

    inline float RectBottom(const s_rect rect) {
        return rect.y + rect.height;
    }

    inline int RectBottom(const s_rect_i rect) {
        return rect.y + rect.height;
    }

    inline s_rect RectTranslated(const s_rect rect, const s_vec_2d trans) {
        return {rect.x + trans.x, rect.y + trans.y, rect.width, rect.height};
    }

    inline s_rect_i RectTranslated(const s_rect_i rect, const s_vec_2d_i trans) {
        return {rect.x + trans.x, rect.y + trans.y, rect.width, rect.height};
    }

    inline bool DoRectsIntersect(const s_rect a, const s_rect b) {
        return a.x < RectRight(b)
            && RectRight(a) > b.x
            && a.y < RectBottom(b)
            && RectBottom(a) > b.y;
    }

    inline bool DoRectsIntersect(const s_rect_i a, const s_rect_i b) {
        return a.x < RectRight(b)
            && RectRight(a) > b.x
            && a.y < RectBottom(b)
            && RectBottom(a) > b.y;
    }

    inline float Lerp(const float a, const float b, const float t) {
        return a + ((b - a) * t);
    }

    inline s_vec_2d Lerp(const s_vec_2d a, const s_vec_2d b, const float t) {
        return {Lerp(a.x, b.x, t), Lerp(a.y, b.y, t)};
    }
}
