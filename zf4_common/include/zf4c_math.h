#pragma once

#include <cmath>

namespace zf4 {
    constexpr float gk_pi = 3.14159265359f;

    struct Vec2D {
        float x, y;

        constexpr Vec2D operator+(const Vec2D& other) const {
            return {x + other.x, y + other.y};
        }

        constexpr Vec2D operator-(const Vec2D& other) const {
            return {x - other.x, y - other.y};
        }

        constexpr Vec2D operator*(const float scalar) const {
            return {x * scalar, y * scalar};
        }

        constexpr Vec2D operator/(const float scalar) const {
            return {x / scalar, y / scalar};
        }

        Vec2D& operator+=(const Vec2D& other) {
            x += other.x;
            y += other.y;
            return *this;
        }

        Vec2D& operator-=(const Vec2D& other) {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        Vec2D& operator*=(const float scalar) {
            x *= scalar;
            y *= scalar;
            return *this;
        }

        Vec2D& operator/=(const float scalar) {
            x /= scalar;
            y /= scalar;
            return *this;
        }

        constexpr bool operator==(const Vec2D& other) const {
            return x == other.x && y == other.y;
        }

        constexpr bool operator!=(const Vec2D& other) const {
            return x != other.x || y != other.y;
        }
    };

    union Vec3D {
        struct {
            float x, y, z;
        };

        struct {
            float r, g, b;
        };
    };

    union Vec4D {
        struct {
            float x, y, z, w;
        };

        struct {
            float r, g, b, a;
        };
    };

    struct Pt2D {
        int x, y;
    };

    struct Rect {
        int x, y;
        int width, height;
    };

    struct RectF {
        float x, y;
        float width, height;
    };

    struct Matrix4x4 {
        float elems[4][4];
    };

    void init_identity_matrix_4x4(Matrix4x4* const mat);
    void init_ortho_matrix_4x4(Matrix4x4* const mat, const float left, const float right, const float bottom, const float top, const float near, const float far);

    constexpr float lerp(const float a, const float b, const float t) {
        return a + ((b - a) * t);
    }

    constexpr Vec2D lerp(const Vec2D a, const Vec2D b, const float t) {
        return {lerp(a.x, b.x, t), lerp(a.y, b.y, t)};
    }

    inline float calc_mag(const Vec2D vec) {
        return sqrtf((vec.x * vec.x) + (vec.y * vec.y));
    }

    inline Vec2D calc_normal(const Vec2D vec) {
        const float mag = calc_mag(vec);
        return {vec.x / mag, vec.y / mag};
    }

    inline float calc_dist(const Vec2D a, const Vec2D b) {
        return calc_mag(b - a);
    }

    inline float calc_dir(const Vec2D a, const Vec2D b) {
        return atan2f(-(b.y - a.y), b.x - a.x);
    }

    inline Vec2D calc_len_dir(const float len, const float dir) {
        const Vec2D dirVec = {cosf(dir), -sinf(dir)};
        return dirVec * len;
    }

    constexpr Pt2D get_rect_pos(const Rect& rect) {
        return {rect.x, rect.y};
    }

    constexpr Vec2D get_rect_pos(const RectF& rect) {
        return {rect.x, rect.y};
    }

    constexpr Pt2D get_rect_size(const Rect& rect) {
        return {rect.width, rect.height};
    }

    constexpr Vec2D get_rect_size(const RectF& rect) {
        return {rect.width, rect.height};
    }

    constexpr Pt2D get_rect_center(const Rect& rect) {
        return {rect.x + (rect.width / 2), rect.y + (rect.height / 2)};
    }

    constexpr Vec2D get_rect_center(const RectF& rect) {
        return {rect.x + (rect.width / 2.0f), rect.y + (rect.height / 2.0f)};
    }

    constexpr int get_rect_right(const Rect& rect) {
        return rect.x + rect.width;
    }

    constexpr float get_rect_right(const RectF& rect) {
        return rect.x + rect.width;
    }

    constexpr int get_rect_bottom(const Rect& rect) {
        return rect.y + rect.height;
    }

    constexpr float get_rect_bottom(const RectF& rect) {
        return rect.y + rect.height;
    }

    constexpr bool do_rects_intersect(const Rect& a, const Rect& b) {
        return a.x < get_rect_right(b) && get_rect_right(a) > b.x && a.y < get_rect_bottom(b) && get_rect_bottom(a) > b.y;
    }

    constexpr bool do_rect_intersect(const RectF& a, const RectF& b) {
        return a.x < get_rect_right(b) && get_rect_right(a) > b.x && a.y < get_rect_bottom(b) && get_rect_bottom(a) > b.y;
    }
}
