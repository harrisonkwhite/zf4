#pragma once

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <type_traits>

namespace zf4 {
    template<typename tp_type>
    concept c_trivial_type = std::is_trivial_v<tp_type>;

    typedef unsigned char a_byte;

    template<c_trivial_type tp_type>
    struct s_array {
        tp_type* elems_raw;
        int len;

        tp_type& operator[](const int index) {
            assert(elems_raw);
            assert(index >= 0 && index < len);
            return elems_raw[index];
        }

        tp_type& operator[](const int index) const {
            assert(elems_raw);
            assert(index >= 0 && index < len);
            return elems_raw[index];
        }

        operator s_array<const tp_type>() const {
            return {
                .elems_raw = elems_raw,
                .len = len
            };
        }
    };

    template<c_trivial_type tp_type, int tp_len>
    struct s_static_array {
        tp_type elems_raw[tp_len];

        tp_type& operator[](const int index) {
            assert(index >= 0 && index < tp_len);
            return elems_raw[index];
        }

        const tp_type& operator[](const int index) const {
            assert(index >= 0 && index < tp_len);
            return elems_raw[index];
        }

        operator s_array<tp_type>() {
            return {
                .elems_raw = elems_raw,
                .len = tp_len
            };
        }

        operator s_array<const tp_type>() const {
            return {
                .elems_raw = elems_raw,
                .len = tp_len
            };
        }
    };

    template<c_trivial_type tp_type>
    struct s_array_2d {
        tp_type* elems_raw;
        int width;
        int height;

        const s_array<tp_type> operator[](const int index) const {
            assert(index >= 0 && index < height);

            return {
                .elems_raw = elems_raw + (width * index),
                .len = width
            };
        }

        operator s_array<tp_type>() {
            return {
                .elems_raw = elems_raw,
                .len = width * height
            };
        }

        operator s_array<const tp_type>() const {
            return {
                .elems_raw = elems_raw,
                .len = width * height
            };
        }

        operator s_array_2d<const tp_type>() const {
            return {
                .elems_raw = elems_raw,
                .width = width,
                .height = height
            };
        }
    };

    template<c_trivial_type tp_type>
    struct s_list {
        tp_type* elems_raw;
        int cap;
        int len;

        tp_type& operator[](const int index) {
            assert(elems_raw);
            assert(index >= 0 && index < len);
            return elems_raw[index];
        }

        tp_type& operator[](const int index) const {
            assert(elems_raw);
            assert(index >= 0 && index < len);
            return elems_raw[index];
        }
    };

    template<c_trivial_type tp_type, int tp_cap>
    struct s_static_list {
        tp_type elems_raw[tp_cap];
        int len;

        tp_type& operator[](const int index) {
            assert(index >= 0 && index < len);
            return elems_raw[index];
        }

        const tp_type& operator[](const int index) const {
            assert(index >= 0 && index < len);
            return elems_raw[index];
        }

        operator s_list<tp_type>() {
            return {
                .elems_raw = elems_raw,
                .cap = tp_cap,
                .len = len
            };
        }

        operator s_list<const tp_type>() const {
            return {
                .elems_raw = elems_raw,
                .cap = tp_cap,
                .len = len
            };
        }
    };

    constexpr int BitsToBytes(const int bits);

    template<c_trivial_type tp_type, int tp_len>
    struct s_static_activity_array {
        tp_type elems_raw[tp_len];
        s_static_array<a_byte, BitsToBytes(tp_len)> activity;

        tp_type& operator[](const int index) {
            assert(index >= 0 && index < tp_len);
            assert(IsBitActive(index, activity, tp_len));
            return elems_raw[index];
        }

        const tp_type& operator[](const int index) const {
            assert(index >= 0 && index < tp_len);
            assert(IsBitActive(index, activity, tp_len));
            return elems_raw[index];
        }
    };

    extern const s_static_array<const int, 256> g_first_active_bit_indexes; // For mapping a byte to the index of its first active bit.
    extern const s_static_array<const int, 256> g_first_inactive_bit_indexes; // For mapping a byte to the index of its first inactive bit.
    extern const s_static_array<const int, 256> g_active_bit_cnts; // For mapping a byte to the number of active bits it contains.

    struct s_mem_arena {
        void* buf;
        int size;
        int offs;
    };

    int ActiveBitCnt(const s_array<const a_byte> bytes, const int bit_cnt);
    int InactiveBitCnt(const s_array<const a_byte> bytes, const int bit_cnt);
    int IndexOfFirstActiveBit(const s_array<const a_byte> bytes, const int bit_cnt);
    int IndexOfFirstInactiveBit(const s_array<const a_byte> bytes, const int bit_cnt);
    bool AreAllBitsActive(const s_array<const a_byte> bytes, const int bit_cnt);
    bool AreAllBitsInactive(const s_array<const a_byte> bytes, const int bit_cnt);

    bool InitMemArena(s_mem_arena& arena, const int size);
    void CleanMemArena(s_mem_arena& arena);
    void EmptyMemArena(s_mem_arena& arena);
    void RewindMemArena(s_mem_arena& arena, const int offs);

    void* Push(const int size, const int alignment, s_mem_arena& arena);

    constexpr int BitsToBytes(const int bits) {
        return (bits + 7) & ~7;
    }

    constexpr int BytesToBits(const int bytes) {
        return bytes << 3;
    }

    constexpr int KilobytesToBytes(const int kb) {
        return kb * 1024;
    }

    constexpr int MegabytesToBytes(const int mb) {
        return mb * 1024 * 1024;
    }

    constexpr int GigabytesToBytes(const int gb) {
        return gb * 1024 * 1024 * 1024;
    }

    inline int ToIndex(const int x, const int y, const int width) {
        assert(x >= 0 && x < width && y >= 0 && width > 0);
        return (y * width) + x;
    }

    inline int WrappedIndex(const int i, const int len) {
        assert(len > 0);

        const int mod = i % len;
        return mod >= 0 ? mod : len + mod;
    }

    constexpr bool IsPowerOfTwo(const int n) {
        return !(n & (n - 1));
    }

    inline int AlignForward(const int n, const int alignment) {
        assert(n >= 0);
        assert(IsPowerOfTwo(alignment));
        return (n + alignment - 1) & ~(alignment - 1);
    }

    template<c_trivial_type tp_type>
    inline void ZeroOutStruct(tp_type& st) {
        static_assert(std::is_class_v<tp_type>);
        memset(&st, 0, sizeof(st));
    }

    template<c_trivial_type tp_type>
    inline void ZeroOutArrayElems(const s_array<tp_type> array) {
        memset(array.elems_raw, 0, sizeof(tp_type) * array.len);
    }

    template<c_trivial_type tp_type>
    inline void ZeroOutArrayElems2D(const s_array_2d<tp_type> array) {
        memset(array.elems_raw, 0, sizeof(tp_type) * array.width * array.height);
    }

    template<c_trivial_type tp_type>
    bool IsStructZero(const tp_type& st) {
        static_assert(std::is_class_v<tp_type>);

        const auto bytes = (const a_byte*)&st;

        for (int i = 0; i < sizeof(st); ++i) {
            if (bytes[i]) {
                return false;
            }
        }

        return true;
    }

    template<c_trivial_type tp_type>
    bool AreArrayElemsZero(const s_array<const tp_type> array) {
        for (int i = 0; i < array.len; ++i) {
            if (!IsStructZero(array[i])) {
                return false;
            }
        }

        return true;
    }

    template<c_trivial_type tp_type>
    bool Are2DArrayElemsZero(const s_array_2d<const tp_type> array) {
        for (int y = 0; y < array.height; ++y) {
            for (int x = 0; x < array.width; ++x) {
                if (!IsStructZero(array[y][x])) {
                    return false;
                }
            }
        }

        return true;
    }

    template<c_trivial_type tp_type>
    inline int ArraySizeInBytes(const s_array<const tp_type> array) {
        return array.len * sizeof(tp_type);
    }

    template<c_trivial_type tp_type>
    inline zf4::s_array<tp_type> ToArray(const zf4::s_list<const tp_type> list) {
        return {
            .elems_raw = list.elems_raw,
            .len = list.cap
        };
    }

    template<c_trivial_type tp_type>
    inline void ListAppend(s_list<tp_type>& list, const tp_type& elem) {
        assert(list.len < list.cap);
        list[list.len++] = elem;
    }

    template<c_trivial_type tp_type, int tp_cap>
    inline void ListAppend(s_static_list<tp_type, tp_cap>& list, const tp_type& elem) {
        assert(list.len < tp_cap);
        list[list.len++] = elem;
    }

    template<c_trivial_type tp_type>
    inline void ListPop(s_list<tp_type>& list) {
        assert(list.len > 0);
        --list.len;
    }

    template<c_trivial_type tp_type, int tp_cap>
    inline void ListPop(s_static_list<tp_type, tp_cap>& list) {
        assert(list.len > 0);
        --list.len;
    }

    template<c_trivial_type tp_type>
    inline tp_type& ListEnd(const s_list<tp_type> list) {
        assert(list.len > 0);
        return list[list.len - 1];
    }

    template<c_trivial_type tp_type, int tp_cap>
    inline tp_type& ListEnd(s_static_list<tp_type, tp_cap> list) {
        assert(list.len > 0);
        return list[list.len - 1];
    }

    template<c_trivial_type tp_type>
    inline void EmptyList(s_list<tp_type>& list) {
        list.len = 0;
        memset(list.elems_raw, 0, sizeof(tp_type) * list.cap);
    }

    template<c_trivial_type tp_type>
    inline bool IsListEmpty(const s_list<tp_type> list) {
        return list.len == 0;
    }

    template<c_trivial_type tp_type, int tp_cap>
    inline bool IsListEmpty(const s_static_list<tp_type, tp_cap> list) {
        return list.len == 0;
    }

    template<c_trivial_type tp_type>
    inline bool IsListFull(const s_list<tp_type> list) {
        return list.len == list.cap;
    }

    template<c_trivial_type tp_type, int tp_cap>
    inline bool IsListFull(const s_static_list<tp_type, tp_cap> list) {
        return list.len == tp_cap;
    }

    template<c_trivial_type tp_type, int tp_cap>
    inline void ActivateElem(const int index, s_static_activity_array<tp_type, tp_cap>& array) {
        assert(index >= 0 && index < tp_cap);
        ActivateBit(index, array.activity, tp_cap);
    }

    template<c_trivial_type tp_type, int tp_cap>
    inline void DeactivateElem(const int index, s_static_activity_array<tp_type, tp_cap>& array) {
        assert(index >= 0 && index < tp_cap);
        DeactivateBit(index, array.activity, tp_cap);
    }

    template<c_trivial_type tp_type, int tp_cap>
    inline bool IsElemActive(const int index, const s_static_activity_array<tp_type, tp_cap>& array) {
        assert(index >= 0 && index < tp_cap);
        return IsBitActive(index, array.activity, tp_cap);
    }

    inline void ActivateBit(const int bit_index, const s_array<a_byte> bytes, const int bit_cnt) {
        assert(bit_index >= 0 && bit_index < bit_cnt);
        assert(bit_cnt > 0);
        assert(bit_cnt <= BytesToBits(bytes.len));
        bytes[bit_index / 8] |= 1 << (bit_index % 8);
    }

    inline void DeactivateBit(const int bit_index, const s_array<a_byte> bytes, const int bit_cnt) {
        assert(bit_index >= 0 && bit_index < bit_cnt);
        assert(bit_cnt > 0);
        assert(bit_cnt <= BytesToBits(bytes.len));
        bytes[bit_index / 8] &= ~(1 << (bit_index % 8));
    }

    inline bool IsBitActive(const int bit_index, const s_array<const a_byte> bytes, const int bit_cnt) {
        assert(bit_index >= 0 && bit_index < bit_cnt);
        assert(bit_cnt > 0);
        assert(bit_cnt <= BytesToBits(bytes.len));
        return bytes[bit_index / 8] & (1 << (bit_index % 8));
    }

    template<c_trivial_type tp_type>
    inline tp_type* PushType(s_mem_arena& arena) {
        return static_cast<tp_type*>(Push(sizeof(tp_type), alignof(tp_type), arena));
    }

    template<c_trivial_type tp_type>
    s_array<tp_type> PushArray(const int len, s_mem_arena& arena) {
        assert(len > 0);

        s_array array = {
            .elems_raw = static_cast<tp_type*>(Push(sizeof(tp_type) * len, alignof(tp_type), arena))
        };

        if (array.elems_raw) {
            array.len = len;
        }

        return array;
    }

    template<c_trivial_type tp_type>
    s_array_2d<tp_type> PushArray2D(const int width, const int height, s_mem_arena& arena) {
        assert(width > 0 && height > 0);

        s_array_2d array = {
            .elems_raw = static_cast<tp_type*>(Push(sizeof(tp_type) * width * height, alignof(tp_type), arena))
        };

        if (array.elems_raw) {
            array.width = width;
            array.height = height;
        }

        return array;
    }

    template<c_trivial_type tp_type>
    s_list<tp_type> PushList(const int cap, s_mem_arena& arena) {
        assert(cap > 0);

        s_list list = {
            .elems_raw = static_cast<tp_type*>(Push(sizeof(tp_type) * cap, alignof(tp_type), arena))
        };

        if (list.elems_raw) {
            list.cap = cap;
        }

        return list;
    }

    template<c_trivial_type tp_type>
    tp_type* PushArrayRaw(const int len, s_mem_arena& arena) {
        assert(len > 0);
        return static_cast<tp_type*>(Push(sizeof(tp_type) * len, alignof(tp_type), arena));
    }
}
