#pragma once

#include <cmath>

namespace zf4 {
    constexpr float gk_pi = 3.14159265359f;

    struct Vec2D {
        float x;
        float y;

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

    struct Vec2DI {
        int x;
        int y;

        operator Vec2D() const {
            return {static_cast<float>(x), static_cast<float>(y)};
        }
    };

    struct Vec3D {
        float x;
        float y;
        float z;

        constexpr Vec3D operator+(const Vec3D& other) const {
            return {x + other.x, y + other.y, z + other.z};
        }

        constexpr Vec3D operator-(const Vec3D& other) const {
            return {x - other.x, y - other.y, z - other.z};
        }

        constexpr Vec3D operator*(const float scalar) const {
            return {x * scalar, y * scalar, z * scalar};
        }

        constexpr Vec3D operator/(const float scalar) const {
            return {x / scalar, y / scalar, z / scalar};
        }

        Vec3D& operator+=(const Vec3D& other) {
            x += other.x;
            y += other.y;
            z += other.z;
            return *this;
        }

        Vec3D& operator-=(const Vec3D& other) {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            return *this;
        }

        Vec3D& operator*=(const float scalar) {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            return *this;
        }

        Vec3D& operator/=(const float scalar) {
            x /= scalar;
            y /= scalar;
            z /= scalar;
            return *this;
        }

        constexpr bool operator==(const Vec3D& other) const {
            return x == other.x && y == other.y && z == other.z;
        }

        constexpr bool operator!=(const Vec3D& other) const {
            return x != other.x || y != other.y || z != other.z;
        }
    };

    struct Vec3DI {
        int x;
        int y;
        int z;

        operator Vec3D() const {
            return {static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)};
        }
    };

    struct Vec4D {
        float x;
        float y;
        float z;
        float w;

        constexpr Vec4D operator+(const Vec4D& other) const {
            return {x + other.x, y + other.y, z + other.z, w + other.w};
        }

        constexpr Vec4D operator-(const Vec4D& other) const {
            return {x - other.x, y - other.y, z - other.z, w - other.w};
        }

        constexpr Vec4D operator*(const float scalar) const {
            return {x * scalar, y * scalar, z * scalar, w * scalar};
        }

        constexpr Vec4D operator/(const float scalar) const {
            return {x / scalar, y / scalar, z / scalar, w / scalar};
        }

        Vec4D& operator+=(const Vec4D& other) {
            x += other.x;
            y += other.y;
            z += other.z;
            w += other.w;
            return *this;
        }

        Vec4D& operator-=(const Vec4D& other) {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            w -= other.w;
            return *this;
        }

        Vec4D& operator*=(const float scalar) {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            w *= scalar;
            return *this;
        }

        Vec4D& operator/=(const float scalar) {
            x /= scalar;
            y /= scalar;
            z /= scalar;
            w /= scalar;
            return *this;
        }

        constexpr bool operator==(const Vec4D& other) const {
            return x == other.x && y == other.y && z == other.z && w == other.w;
        }

        constexpr bool operator!=(const Vec4D& other) const {
            return x != other.x || y != other.y || z != other.z || w != other.w;
        }
    };

    struct Vec4DI {
        int x;
        int y;
        int z;
        int w;

        operator Vec4D() const {
            return {static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(w)};
        }
    };

    struct Rect {
        float x;
        float y;
        float width;
        float height;
    };

    struct RectI {
        int x;
        int y;
        int width;
        int height;

        operator Rect() const {
            return {static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height)};
        }
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

    constexpr Vec3D lerp(const Vec3D a, const Vec3D b, const float t) {
        return {lerp(a.x, b.x, t), lerp(a.y, b.y, t), lerp(a.z, b.z, t)};
    }

    constexpr Vec4D lerp(const Vec4D a, const Vec4D b, const float t) {
        return {lerp(a.x, b.x, t), lerp(a.y, b.y, t), lerp(a.z, b.z, t), lerp(a.w, b.w, t)};
    }

    inline float calc_mag(const Vec2D vec) {
        return sqrtf((vec.x * vec.x) + (vec.y * vec.y));
    }

    inline float calc_mag(const Vec3D vec) {
        return sqrtf((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z));
    }

    inline float calc_mag(const Vec4D vec) {
        return sqrtf((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z) + (vec.w * vec.w));
    }

    inline Vec2D calc_normal(const Vec2D vec) {
        const float mag = calc_mag(vec);
        return {vec.x / mag, vec.y / mag};
    }

    inline Vec3D calc_normal(const Vec3D vec) {
        const float mag = calc_mag(vec);
        return {vec.x / mag, vec.y / mag, vec.z / mag};
    }

    inline Vec4D calc_normal(const Vec4D vec) {
        const float mag = calc_mag(vec);
        return {vec.x / mag, vec.y / mag, vec.z / mag, vec.w / mag};
    }

    inline float calc_dist(const Vec2D a, const Vec2D b) {
        return calc_mag(b - a);
    }

    inline float calc_dist(const Vec3D a, const Vec3D b) {
        return calc_mag(b - a);
    }

    inline float calc_dist(const Vec4D a, const Vec4D b) {
        return calc_mag(b - a);
    }

    inline float calc_dir(const Vec2D a, const Vec2D b) {
        return atan2f(-(b.y - a.y), b.x - a.x);
    }

    inline Vec2D calc_len_dir(const float len, const float dir) {
        const Vec2D dirVec = {cosf(dir), -sinf(dir)};
        return dirVec * len;
    }

    constexpr Vec2D get_rect_pos(const Rect& rect) {
        return {rect.x, rect.y};
    }

    constexpr Vec2DI get_rect_pos(const RectI& rect) {
        return {rect.x, rect.y};
    }

    constexpr Vec2D get_rect_size(const Rect& rect) {
        return {rect.width, rect.height};
    }

    constexpr Vec2DI get_rect_size(const RectI& rect) {
        return {rect.width, rect.height};
    }

    constexpr Vec2D get_rect_center(const Rect& rect) {
        return {rect.x + (rect.width / 2.0f), rect.y + (rect.height / 2.0f)};
    }

    constexpr Vec2DI get_rect_center(const RectI& rect) {
        return {rect.x + (rect.width / 2), rect.y + (rect.height / 2)};
    }

    constexpr float get_rect_right(const Rect& rect) {
        return rect.x + rect.width;
    }

    constexpr int get_rect_right(const RectI& rect) {
        return rect.x + rect.width;
    }

    constexpr float get_rect_bottom(const Rect& rect) {
        return rect.y + rect.height;
    }

    constexpr int get_rect_bottom(const RectI& rect) {
        return rect.y + rect.height;
    }

    constexpr bool do_rect_intersect(const Rect& a, const Rect& b) {
        return a.x < get_rect_right(b) && get_rect_right(a) > b.x && a.y < get_rect_bottom(b) && get_rect_bottom(a) > b.y;
    }

    constexpr bool do_rects_intersect(const RectI& a, const RectI& b) {
        return a.x < get_rect_right(b) && get_rect_right(a) > b.x && a.y < get_rect_bottom(b) && get_rect_bottom(a) > b.y;
    }
}
