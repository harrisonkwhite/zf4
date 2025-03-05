#pragma once

#include <cmath>
#include <cassert>
#include <zf4_mem.h>

namespace zf4 {
    constexpr float g_pi = 3.14159265359f;

    struct s_vec_2d {
        float x = 0.0f;
        float y = 0.0f;

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
        int x = 0;
        int y = 0;

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
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;

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
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float w = 0.0f;

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

    struct s_rect {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;

        float Right() const {
            return x + width;
        }

        float Bottom() const {
            return y + height;
        }

        s_vec_2d Size() const {
            return {width, height};
        }

        s_vec_2d TopLeft() const {
            return {x, y};
        }

        s_vec_2d TopCenter() const {
            return {x + (width / 2.0f), y};
        }

        s_vec_2d TopRight() const {
            return {x + width, y};
        }

        s_vec_2d CenterLeft() const {
            return {x, y + (height / 2.0f)};
        }

        s_vec_2d Center() const {
            return {x + (width / 2.0f), y + (height / 2.0f)};
        }

        s_vec_2d CenterRight() const {
            return {x + width, y + (height / 2.0f)};
        }

        s_vec_2d BottomLeft() const {
            return {x, y + height};
        }

        s_vec_2d BottomCenter() const {
            return {x + (width / 2.0f), y + height};
        }

        s_vec_2d BottomRight() const {
            return {x + width, y + height};
        }
    };

    struct s_rect_i {
        int x;
        int y;
        int width;
        int height;

        operator s_rect() const {
            return s_rect(x, y, width, height);
        }

        int Right() const {
            return x + width;
        }

        int Bottom() const {
            return y + height;
        }

        s_vec_2d_i Size() const {
            return {width, height};
        }

        s_vec_2d_i TopLeft() const {
            return {x, y};
        }

        s_vec_2d_i TopCenter() const {
            return {x + (width / 2), y};
        }

        s_vec_2d_i TopRight() const {
            return {x + width, y};
        }

        s_vec_2d_i CenterLeft() const {
            return {x, y + (height / 2)};
        }

        s_vec_2d_i Center() const {
            return {x + (width / 2), y + (height / 2)};
        }

        s_vec_2d_i CenterRight() const {
            return {x + width, y + (height / 2)};
        }

        s_vec_2d_i BottomLeft() const {
            return {x, y + height};
        }

        s_vec_2d_i BottomCenter() const {
            return {x + (width / 2), y + height};
        }

        s_vec_2d_i BottomRight() const {
            return {x + width, y + height};
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

    inline bool InBounds(const s_vec_2d pos, const s_vec_2d size) {
        assert(size.x > 0 && size.y > 0);
        return pos.x >= 0.0f && pos.y >= 0.0f && pos.x < size.x && pos.y < size.y;
    }

    inline bool InBounds(const s_vec_2d_i pos, const s_vec_2d_i size) {
        assert(size.x > 0 && size.y > 0);
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

    inline float DistSquared(const s_vec_2d a, const s_vec_2d b) {
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

#if 0
    inline bool IsPointInRect(const zf4::s_vec_2d pt, const s_rect rect) {
        return pt.x >= rect.x && pt.x < RectRight(rect)
            && pt.y >= rect.y && pt.y < RectBottom(rect);
    }

    inline bool IsPointInRect(const zf4::s_vec_2d_i pt, const s_rect_i rect) {
        return pt.x >= rect.x && pt.x < RectRight(rect)
            && pt.y >= rect.y && pt.y < RectBottom(rect);
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
#endif

    inline float Lerp(const float a, const float b, const float t) {
        return a + ((b - a) * t);
    }

    inline s_vec_2d Lerp(const s_vec_2d a, const s_vec_2d b, const float t) {
        return {Lerp(a.x, b.x, t), Lerp(a.y, b.y, t)};
    }

    inline int Clamp(const int val, const int min, const int max) {
        if (val < min) {
            return min;
        } else if (val > max) {
            return max;
        } else {
            return val;
        }
    }

    inline float Clamp(const float val, const float min, const float max) {
        if (val < min) {
            return min;
        } else if (val > max) {
            return max;
        } else {
            return val;
        }
    }
}
