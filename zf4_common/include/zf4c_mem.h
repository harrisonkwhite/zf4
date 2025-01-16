#pragma once

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <type_traits>

namespace zf4 {
    // For mapping a byte to the index of its first active bit.
    constexpr int gk_firstActiveBitIndexes[256] = {
        -1, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
    };

    // For mapping a byte to the index of its first inactive bit.
    constexpr int gk_firstInactiveBitIndexes[256] = {
        0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
        0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
        0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
        0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6,
        0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
        0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
        0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
        0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 7,
        0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
        0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
        0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
        0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6,
        0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
        0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
        0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
        0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 8
    };

    // For mapping a byte to the number of active bits it contains.
    constexpr int gk_activeBitCnts[256] = {
        0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
    };

    using Byte = unsigned char;

    template <typename T>
    concept SimpleType = std::is_trivial_v<T> && std::is_standard_layout_v<T>;

    template<SimpleType T>
    class SafePtr {
    public:
        SafePtr() = default;

#ifdef _DEBUG
        SafePtr(T* const elems, const int elemCnt) : m_elems(elems), m_elemCnt(elemCnt) {
        }
#else
        SafePtr(T* const elems, const int elemCnt) : m_elems(elems) {
        }
#endif

        T* get() {
            return m_elems; // TEMP?
        }

        const T* get() const {
            return m_elems; // TEMP?
        }

        T& operator[](const int index) {
            assert(index >= 0 && index < m_elemCnt);
            return m_elems[index];
        }

        const T& operator[](const int index) const {
            assert(index >= 0 && index < m_elemCnt);
            return m_elems[index];
        }

        bool operator()() const {
            return m_elems;
        }

        bool operator!() const {
            return !m_elems;
        }

    private:
        T* m_elems;

#ifdef _DEBUG
        int m_elemCnt; // We only hold this additional metadata in debug mode so we can catch any accidental out-of-bounds accesses.
#endif
    };

    class MemArena {
    public:
        bool init(const int size);
        void clean();
        SafePtr<Byte> alloc(const int size, const int alignment);
        template<SimpleType T> SafePtr<T> alloc(const int cnt = 1);
        void reset();

    private:
        Byte* m_bytes;
        int m_size;
        int m_offs;
    };

    int get_active_bit_cnt(const Byte* const bytes, const int byteCnt);
    int get_inactive_bit_cnt(const Byte* const bytes, const int byteCnt);

    int get_first_active_bit_index(const Byte* const bytes, const int byteCnt);
    int get_first_inactive_bit_index(const Byte* const bytes, const int byteCnt);

    bool are_all_bits_active(const Byte* const bytes, const int byteCnt);
    bool are_all_bits_inactive(const Byte* const bytes, const int byteCnt);

    constexpr int kilobytes_to_bytes(const int kbs) {
        return kbs * 1024;
    }

    constexpr int megabytes_to_bytes(const int mbs) {
        return mbs * 1024 * 1024;
    }

    constexpr int gigabytes_to_bytes(const int gbs) {
        return gbs * 1024 * 1024 * 1024;
    }

    constexpr int bits_to_bytes(const int bits) {
        return (bits + 7) & ~7;
    }

    template<SimpleType T>
    T* alloc(const int cnt = 1) {
        return static_cast<T*>(std::malloc(sizeof(T) * cnt));
    }

    template<SimpleType T>
    T* alloc_zeroed(const int cnt = 1) {
        return static_cast<T*>(std::calloc(cnt, sizeof(T)));
    }

    template<SimpleType T>
    bool is_zero(const T* const data, const int cnt = 1) {
        static_assert(!std::is_pointer_v<T>);
        assert(cnt > 0);

        const auto bytes = reinterpret_cast<const Byte*>(data);

        for (int i = 0; i < sizeof(T) * cnt; ++i) {
            if (bytes[i]) {
                return false;
            }
        }

        return true;
    }

    template<SimpleType T>
    void zero_out(T* const data, const int cnt = 1) {
        static_assert(!std::is_pointer_v<T>);
        std::memset(data, 0, sizeof(T) * cnt);
    }

    inline bool is_power_of_two(const int n) {
        return !(n & (n - 1));
    }

    inline int align_forward(const int n, const int alignment) {
        assert(n >= 0);
        assert(is_power_of_two(alignment));
        return (n + alignment - 1) & ~(alignment - 1);
    }

    inline void activate_bit(Byte* const bytes, const int bitIndex) {
        bytes[bitIndex / 8] |= 1 << (bitIndex % 8);
    }

    inline void deactivate_bit(Byte* const bytes, const int bitIndex) {
        bytes[bitIndex / 8] &= ~(1 << (bitIndex % 8));
    }

    inline void clear_bits(Byte* const bytes, const int bitCnt) {
        memset(bytes, 0, bits_to_bytes(bitCnt));
    }

    inline bool is_bit_active(const Byte* const bytes, const int bitIndex) {
        return bytes[bitIndex / 8] & (1 << (bitIndex % 8));
    }

    template<SimpleType T>
    inline SafePtr<T> MemArena::alloc(const int cnt) {
        assert(m_bytes);
        assert(cnt > 0);

        const int size = sizeof(T) * cnt;

        const int offsAligned = align_forward(m_offs, alignof(T));
        const int offsNext = offsAligned + size;

        if (offsNext > m_size) {
            return {};
        }

        m_offs = offsNext;

        return {reinterpret_cast<T*>(m_bytes + offsAligned), size};
    }
}
