#ifndef ZF4C_MEM_H
#define ZF4C_MEM_H

#include <cassert>
#include <cstdlib>
#include <cstdbool>
#include <cstring>
#include <type_traits>

#define ZF4_KILOBYTES(X) (X * 1024)
#define ZF4_MEGABYTES(X) (X * 1024 * 1024)
#define ZF4_GIGABYTES(X) (X * 1024 * 1024 * 1024)
#define ZF4_BITS_TO_BYTES(X) ((X + 7) & ~7)

using ZF4Byte = unsigned char;

template <typename T>
concept ZF4SimpleType = std::is_trivial_v<T> && std::is_standard_layout_v<T>;

class ZF4MemArena {
public:
    bool init(const int size);
    void clean();
    ZF4Byte* push(const int size, const int alignment);
    void reset();

    template<ZF4SimpleType T>
    T* push(const int cnt = 1) {
        return reinterpret_cast<T*>(push(sizeof(T) * cnt, alignof(T)));
    }

private:
    ZF4Byte* bytes;
    int size;
    int offs;
};

int zf4_get_first_active_bit_index(const ZF4Byte* const bytes, const int bitCnt);
int zf4_get_first_inactive_bit_index(const ZF4Byte* const bytes, const int bitCnt);
bool zf4_are_all_bits_active(const ZF4Byte* const bytes, const int bitCnt);
bool zf4_are_all_bits_inactive(const ZF4Byte* const bytes, const int bitCnt);

template<ZF4SimpleType T>
T* zf4_alloc(const int cnt = 1) {
    return static_cast<T*>(std::malloc(sizeof(T) * cnt));
}

template<ZF4SimpleType T>
T* zf4_alloc_zeroed(const int cnt = 1) {
    return static_cast<T*>(std::calloc(cnt, sizeof(T)));
}

template<ZF4SimpleType T>
bool zf4_is_zero(const T* const data) {
    const auto bytes = reinterpret_cast<const ZF4Byte*>(data);

    for (int i = 0; i < sizeof(T); ++i) {
        if (bytes[i]) {
            return false;
        }
    }

    return true;
}

template<ZF4SimpleType T>
void zf4_zero_out(T* const data, const int cnt = 1) {
    std::memset(data, 0, sizeof(T) * cnt);
}

inline bool zf4_is_power_of_two(const int n) {
    return !(n & (n - 1));
}

inline int zf4_align_forward(const int n, const int alignment) {
    assert(n >= 0);
    assert(zf4_is_power_of_two(alignment));
    return (n + alignment - 1) & ~(alignment - 1);
}

inline void zf4_activate_bit(ZF4Byte* const bytes, const int bitIndex) {
    bytes[bitIndex / 8] |= 1 << (bitIndex % 8);
}

inline void zf4_deactivate_bit(ZF4Byte* const bytes, const int bitIndex) {
    bytes[bitIndex / 8] &= ~(1 << (bitIndex % 8));
}

inline void zf4_clear_bits(ZF4Byte* const bytes, const int bitCnt) {
    memset(bytes, 0, ZF4_BITS_TO_BYTES(bitCnt));
}

inline bool zf4_is_bit_active(const ZF4Byte* const bytes, const int bitIndex) {
    return bytes[bitIndex / 8] & (1 << (bitIndex % 8));
}

#endif
