#pragma once

#include <cmath>
#include <cassert>
#include <zf4c_mem.h>

namespace zf4 {
    constexpr float g_pi = 3.14159265359f;

    struct s_vec_2d {
        float x;
        float y;

        constexpr s_vec_2d operator+(const s_vec_2d other) const {
            return {x + other.x, y + other.y};
        }

        constexpr s_vec_2d operator-(const s_vec_2d other) const {
            return {x - other.x, y - other.y};
        }

        constexpr s_vec_2d operator*(const float scalar) const {
            return {x * scalar, y * scalar};
        }

        constexpr s_vec_2d operator/(const float scalar) const {
            return {x / scalar, y / scalar};
        }

        s_vec_2d& operator+=(const s_vec_2d other) {
            x += other.x;
            y += other.y;
            return *this;
        }

        s_vec_2d& operator-=(const s_vec_2d other) {
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

        constexpr bool operator==(const s_vec_2d other) const {
            return x == other.x && y == other.y;
        }

        constexpr bool operator!=(const s_vec_2d other) const {
            return x != other.x || y != other.y;
        }

        constexpr s_vec_2d operator-() const {
            return {-x, -y};
        }
    };

    struct s_vec_2d_i {
        int x;
        int y;

        constexpr s_vec_2d_i operator+(const s_vec_2d_i other) const {
            return {x + other.x, y + other.y};
        }

        constexpr s_vec_2d operator+(const s_vec_2d other) const {
            return {x + other.x, y + other.y};
        }

        constexpr s_vec_2d_i operator-(const s_vec_2d_i other) const {
            return {x - other.x, y - other.y};
        }

        constexpr s_vec_2d operator-(const s_vec_2d other) const {
            return {x - other.x, y - other.y};
        }

        constexpr s_vec_2d_i operator*(const int scalar) const {
            return {x * scalar, y * scalar};
        }

        constexpr s_vec_2d operator*(const float scalar) const {
            return {x * scalar, y * scalar};
        }

        constexpr s_vec_2d_i operator/(const int scalar) const {
            return {x / scalar, y / scalar};
        }

        constexpr s_vec_2d operator/(const float scalar) const {
            return {x / scalar, y / scalar};
        }

        s_vec_2d_i& operator+=(const s_vec_2d_i other) {
            x += other.x;
            y += other.y;
            return *this;
        }

        s_vec_2d_i& operator-=(const s_vec_2d_i other) {
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

        constexpr bool operator==(const s_vec_2d_i other) const {
            return x == other.x && y == other.y;
        }

        constexpr bool operator!=(const s_vec_2d_i other) const {
            return x != other.x || y != other.y;
        }

        constexpr s_vec_2d_i operator-() const {
            return {-x, -y};
        }

        constexpr operator s_vec_2d() const {
            return {static_cast<float>(x), static_cast<float>(y)};
        }
    };

    struct s_vec_3d {
        float x;
        float y;
        float z;

        constexpr s_vec_3d operator+(const s_vec_3d other) const {
            return {x + other.x, y + other.y, z + other.z};
        }

        constexpr s_vec_3d operator-(const s_vec_3d other) const {
            return {x - other.x, y - other.y, z - other.z};
        }

        constexpr s_vec_3d operator*(const float scalar) const {
            return {x * scalar, y * scalar, z * scalar};
        }

        constexpr s_vec_3d operator/(const float scalar) const {
            return {x / scalar, y / scalar, z / scalar};
        }

        s_vec_3d& operator+=(const s_vec_3d other) {
            x += other.x;
            y += other.y;
            z += other.z;
            return *this;
        }

        s_vec_3d& operator-=(const s_vec_3d other) {
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

        constexpr bool operator==(const s_vec_3d other) const {
            return x == other.x && y == other.y && z == other.z;
        }

        constexpr bool operator!=(const s_vec_3d other) const {
            return x != other.x || y != other.y || z != other.z;
        }

        constexpr s_vec_3d operator-() const {
            return {-x, -y, -z};
        }
    };

    struct s_vec_4d {
        float x;
        float y;
        float z;
        float w;

        constexpr s_vec_4d operator+(const s_vec_4d other) const {
            return {x + other.x, y + other.y, z + other.z, w + other.w};
        }

        constexpr s_vec_4d operator-(const s_vec_4d other) const {
            return {x - other.x, y - other.y, z - other.z, w - other.w};
        }

        constexpr s_vec_4d operator*(const float scalar) const {
            return {x * scalar, y * scalar, z * scalar, w * scalar};
        }

        constexpr s_vec_4d operator/(const float scalar) const {
            return {x / scalar, y / scalar, z / scalar, w / scalar};
        }

        s_vec_4d& operator+=(const s_vec_4d other) {
            x += other.x;
            y += other.y;
            z += other.z;
            w += other.w;
            return *this;
        }

        s_vec_4d& operator-=(const s_vec_4d other) {
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

        constexpr bool operator==(const s_vec_4d other) const {
            return x == other.x && y == other.y && z == other.z && w == other.w;
        }

        constexpr bool operator!=(const s_vec_4d other) const {
            return x != other.x || y != other.y || z != other.z || w != other.w;
        }

        constexpr s_vec_4d operator-() const {
            return {-x, -y, -z, -w};
        }
    };

    struct s_poly_view {
        s_array<const s_vec_2d> pts;
    };

    struct s_poly {
        s_array<s_vec_2d> pts;

        operator s_poly_view() const {
            return {
                .pts = pts
            };
        }
    };

    struct s_range {
        int min;
        int max;
    };

    struct s_range_f {
        float min;
        float max;
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

        operator s_rect() const {
            return s_rect(x, y, width, height);
        }
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

    bool DoPolysIntersect(const s_poly_view poly_a, const s_poly_view poly_b);
    s_poly PushQuadPoly(const s_vec_2d pos, const s_vec_2d size, const s_vec_2d origin, s_mem_arena& mem_arena);
    s_poly PushRotatedQuadPoly(const s_vec_2d pos, const s_vec_2d size, const s_vec_2d origin, const float rot, s_mem_arena& mem_arena);
    bool DoesPolyIntersectWithRect(const s_poly_view poly, const s_rect rect);
    float PolyLeft(const s_poly_view poly);
    float PolyRight(const s_poly_view poly);
    float PolyTop(const s_poly_view poly);
    float PolyBottom(const s_poly_view poly);
    bool IsPolyQuad(const s_poly_view poly);
    bool IsPolyQuadNonRot(const s_poly_view poly);

    s_matrix_4x4 GenIdentityMatrix4x4();
    s_matrix_4x4 GenOrthoMatrix4x4(const float left, const float right, const float bottom, const float top, const float near, const float far);

    constexpr bool InBounds(const s_vec_2d pos, const s_vec_2d size) {
        return pos.x >= 0.0f && pos.y >= 0.0f && pos.x < size.x && pos.y < size.y;
    }

    constexpr bool InBounds(const s_vec_2d_i pos, const s_vec_2d_i size) {
        return pos.x >= 0 && pos.y >= 0 && pos.x < size.x && pos.y < size.y;
    }

    inline zf4::s_vec_2d_i Snapped(const zf4::s_vec_2d pos, const int cell_size) {
        assert(cell_size > 0);

        return {
            static_cast<int>(floor(pos.x / cell_size)),
            static_cast<int>(floor(pos.y / cell_size))
        };
    }

    constexpr int Min(const int a, const int b) {
        return a < b ? a : b;
    }

    constexpr float Min(const float a, const float b) {
        return a < b ? a : b;
    }

    constexpr int Max(const int a, const int b) {
        return a > b ? a : b;
    }

    constexpr float Max(const float a, const float b) {
        return a > b ? a : b;
    }

    constexpr int Sign(const int val) {
        return val < 0 ? -1 : 1;
    }

    constexpr int Sign(const float val) {
        return val < 0.0f ? -1 : 1;
    }

    constexpr float DegsToRads(const float degs) {
        return degs * (g_pi / 180.0f);
    }

    constexpr float RadsToDegs(const float rads) {
        return rads * (180.0f / g_pi);
    }

    inline float Magnitude(const s_vec_2d vec) {
        return sqrt((vec.x * vec.x) + (vec.y * vec.y));
    }

    inline s_vec_2d Normal(const s_vec_2d vec) {
        const float mag = Magnitude(vec);
        assert(mag != 0.0f);
        return vec * (1.0f / mag);
    }

    inline float Dot(const s_vec_2d a, const s_vec_2d b) {
        return (a.x * b.x) + (a.y * b.y);
    }

    inline s_vec_2d Midpoint(const s_vec_2d a, const s_vec_2d b) {
        return (a + b) / 2.0f;
    }

    inline float Dist(const s_vec_2d a, const s_vec_2d b) {
        return Magnitude(b - a);
    }

    constexpr float DistSquared(const s_vec_2d a, const s_vec_2d b) {
        const float x_diff = b.x - a.x;
        const float y_diff = b.y - a.y;
        return (x_diff * x_diff) + (y_diff * y_diff);
    }

    inline float Dir(const s_vec_2d vec) {
        return atan2(-vec.y, vec.x);
    }

    inline float Dir(const s_vec_2d a, const s_vec_2d b) {
        return atan2(-(b.y - a.y), b.x - a.x);
    }

    inline s_vec_2d DirVec(const float dir) {
        return {cos(dir), -sin(dir)};
    }

    inline s_vec_2d LenDir(const float len, const float dir) {
        return {len * cos(dir), -len * sin(dir)};
    }

    constexpr s_rect GenRect(const s_vec_2d pos, const s_vec_2d size) {
        return {pos.x, pos.y, size.x, size.y};
    }

    constexpr s_rect_i GenRect(const s_vec_2d_i pos, const s_vec_2d_i size) {
        return {pos.x, pos.y, size.x, size.y};
    }

    constexpr s_vec_2d RectSize(const s_rect rect) {
        return {rect.width, rect.height};
    }

    constexpr s_vec_2d_i RectSize(const s_rect_i rect) {
        return {rect.width, rect.height};
    }

    constexpr float RectRight(const s_rect rect) {
        return rect.x + rect.width;
    }

    constexpr int RectRight(const s_rect_i rect) {
        return rect.x + rect.width;
    }

    constexpr float RectBottom(const s_rect rect) {
        return rect.y + rect.height;
    }

    constexpr int RectBottom(const s_rect_i rect) {
        return rect.y + rect.height;
    }

    constexpr s_vec_2d RectTopLeft(const s_rect rect) {
        return {rect.x, rect.y};
    }

    constexpr s_vec_2d_i RectTopLeft(const s_rect_i rect) {
        return {rect.x, rect.y};
    }

    constexpr s_vec_2d RectTopCenter(const s_rect rect) {
        return {rect.x + (rect.width / 2.0f), rect.y};
    }

    constexpr s_vec_2d_i RectTopCenter(const s_rect_i rect) {
        return {rect.x + (rect.width / 2), rect.y};
    }

    constexpr s_vec_2d RectTopRight(const s_rect rect) {
        return {RectRight(rect), rect.y};
    }

    constexpr s_vec_2d_i RectTopRight(const s_rect_i rect) {
        return {RectRight(rect), rect.y};
    }

    constexpr s_vec_2d RectCenterLeft(const s_rect rect) {
        return {rect.x, rect.y + (rect.height / 2.0f)};
    }

    constexpr s_vec_2d_i RectCenterLeft(const s_rect_i rect) {
        return {rect.x, rect.y + (rect.height / 2)};
    }

    constexpr s_vec_2d RectCenter(const s_rect rect) {
        return {rect.x + (rect.width / 2.0f), rect.y + (rect.height / 2.0f)};
    }

    constexpr s_vec_2d_i RectCenter(const s_rect_i rect) {
        return {rect.x + (rect.width / 2), rect.y + (rect.height / 2)};
    }

    constexpr s_vec_2d RectCenterRight(const s_rect rect) {
        return {RectRight(rect), rect.y + (rect.height / 2.0f)};
    }

    constexpr s_vec_2d_i RectCenterRight(const s_rect_i rect) {
        return {RectRight(rect), rect.y + (rect.height / 2)};
    }

    constexpr s_vec_2d RectBottomLeft(const s_rect rect) {
        return {rect.x, RectBottom(rect)};
    }

    constexpr s_vec_2d_i RectBottomLeft(const s_rect_i rect) {
        return {rect.x, RectBottom(rect)};
    }

    constexpr s_vec_2d RectBottomCenter(const s_rect rect) {
        return {rect.x + (rect.width / 2.0f), RectBottom(rect)};
    }

    constexpr s_vec_2d_i RectBottomCenter(const s_rect_i rect) {
        return {rect.x + (rect.width / 2), RectBottom(rect)};
    }

    constexpr s_vec_2d RectBottomRight(const s_rect rect) {
        return {RectRight(rect), RectBottom(rect)};
    }

    constexpr s_vec_2d_i RectBottomRight(const s_rect_i rect) {
        return {RectRight(rect), RectBottom(rect)};
    }

    constexpr s_rect RectTranslated(const s_rect rect, const s_vec_2d trans) {
        return {rect.x + trans.x, rect.y + trans.y, rect.width, rect.height};
    }

    constexpr s_rect_i RectTranslated(const s_rect_i rect, const s_vec_2d_i trans) {
        return {rect.x + trans.x, rect.y + trans.y, rect.width, rect.height};
    }

    constexpr bool IsPointInRect(const zf4::s_vec_2d pt, const s_rect rect) {
        return pt.x >= rect.x && pt.x < RectRight(rect)
            && pt.y >= rect.y && pt.y < RectBottom(rect);
    }

    constexpr bool IsPointInRect(const zf4::s_vec_2d_i pt, const s_rect_i rect) {
        return pt.x >= rect.x && pt.x < RectRight(rect)
            && pt.y >= rect.y && pt.y < RectBottom(rect);
    }

    constexpr bool DoRectsIntersect(const s_rect a, const s_rect b) {
        return a.x < RectRight(b)
            && RectRight(a) > b.x
            && a.y < RectBottom(b)
            && RectBottom(a) > b.y;
    }

    constexpr bool DoRectsIntersect(const s_rect_i a, const s_rect_i b) {
        return a.x < RectRight(b)
            && RectRight(a) > b.x
            && a.y < RectBottom(b)
            && RectBottom(a) > b.y;
    }

    constexpr float Lerp(const float a, const float b, const float t) {
        return a + ((b - a) * t);
    }

    constexpr s_vec_2d Lerp(const s_vec_2d a, const s_vec_2d b, const float t) {
        return {Lerp(a.x, b.x, t), Lerp(a.y, b.y, t)};
    }

    constexpr int Clamp(const int val, const int min, const int max) {
        if (val < min) {
            return min;
        } else if (val > max) {
            return max;
        } else {
            return val;
        }
    }

    constexpr float Clamp(const float val, const float min, const float max) {
        if (val < min) {
            return min;
        } else if (val > max) {
            return max;
        } else {
            return val;
        }
    }
}
