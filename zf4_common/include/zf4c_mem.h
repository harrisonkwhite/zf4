#pragma once

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <type_traits>

namespace zf4 {
    template<typename tp_type>
    concept c_simple_type = std::is_trivial_v<tp_type> && std::is_standard_layout_v<tp_type>;

    typedef unsigned char a_byte;

    template<c_simple_type tp_type>
    struct s_array {
        tp_type* elems_raw;
        int len;

        tp_type& operator[](const int index) {
            assert(index >= 0 && index < len);
            return elems_raw[index];
        }

        tp_type& operator[](const int index) const {
            assert(index >= 0 && index < len);
            return elems_raw[index];
        }
    };

    template<c_simple_type tp_type, int tp_len>
    struct s_static_array {
        static constexpr int len = tp_len;

        tp_type elems_raw[tp_len];

        tp_type& operator[](const int index) {
            assert(index >= 0 && index < tp_len);
            return elems_raw[index];
        }

        const tp_type& operator[](const int index) const {
            assert(index >= 0 && index < tp_len);
            return elems_raw[index];
        }
    };

    template<c_simple_type tp_type, int tp_cap>
    struct s_static_list {
        static constexpr int cap = tp_cap;

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

    constexpr bool IsPowerOfTwo(const int n) {
        return !(n & (n - 1));
    }

    inline int AlignForward(const int n, const int alignment) {
        assert(n >= 0);
        assert(IsPowerOfTwo(alignment));
        return (n + alignment - 1) & ~(alignment - 1);
    }

    template<c_simple_type tp_type>
    inline void ZeroOutStruct(tp_type& st) {
        static_assert(std::is_class_v<tp_type>);
        memset(&st, 0, sizeof(st));
    }

    template<c_simple_type tp_type>
    inline void ZeroOutArrayElems(const s_array<tp_type> array) {
        memset(array.elems_raw, 0, sizeof(tp_type) * array.len);
    }

    template<c_simple_type tp_type, int tp_len>
    inline void ZeroOutArrayElems(s_static_array<tp_type, tp_len>& array) {
        memset(array.elems_raw, 0, sizeof(tp_type) * tp_len);
    }

    template<c_simple_type tp_type>
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

    template<c_simple_type tp_type>
    inline s_array<const tp_type> CreateView(const s_array<tp_type> array) {
        return {
            .elems_raw = array.elems_raw,
            .len = array.len
        };
    }

    template<c_simple_type tp_type>
    inline int ArraySizeInBytes(const s_array<const tp_type> array) {
        return sizeof(tp_type) * array.len;
    }

    template<c_simple_type tp_type, int tp_len>
    inline s_array<tp_type> StaticArrayToArray(s_static_array<tp_type, tp_len>& static_array) {
        return {
            .elems_raw = (tp_type*)static_array.elems_raw,
            .len = tp_len
        };
    }

    template<c_simple_type tp_type, int tp_len>
    inline s_array<const tp_type> StaticArrayToArray(const s_static_array<tp_type, tp_len>& static_array) {
        return {
            .elems_raw = (const tp_type*)static_array.elems_raw,
            .len = tp_len
        };
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

    template<c_simple_type tp_type>
    inline tp_type* PushType(s_mem_arena& arena) {
        return static_cast<tp_type*>(Push(sizeof(tp_type), alignof(tp_type), arena));
    }

    template<c_simple_type tp_type>
    s_array<tp_type> PushArray(const int len, s_mem_arena& arena) {
        s_array array = {
            .elems_raw = static_cast<tp_type*>(Push(sizeof(tp_type) * len, alignof(tp_type), arena))
        };

        if (array.elems_raw) {
            array.len = len;
        }

        return array;
    }
}
