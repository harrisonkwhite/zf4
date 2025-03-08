#pragma once

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <type_traits>
#include <initializer_list>
#include <new>

namespace zf4 {
    typedef unsigned char a_byte;

    template<typename tp_type>
    struct s_array {
        tp_type* elems_raw = nullptr;
        size_t len = 0;

        s_array() = default;

        s_array(tp_type* const elems, const size_t len) {
            assert(elems && len > 0);

            elems_raw = elems;
            this->len = len;
        }

        tp_type& operator[](const size_t index) {
            assert(IsValid());
            assert(index < len);
            return elems_raw[index];
        }

        tp_type& operator[](const size_t index) const {
            assert(IsValid());
            assert(index < len);
            return elems_raw[index];
        }

        operator s_array<const tp_type>() const {
            return {elems_raw, len};
        }

        bool IsValid() const {
            return (!elems_raw && len == 0) || (elems_raw && len > 0);
        }

        bool IsInitialized() const {
            assert(IsValid());
            return !elems_raw;
        }

        size_t SizeInBytes() const {
            assert(IsValid());
            return len * sizeof(tp_type);
        }
    };

    template<typename tp_type, size_t tp_len>
    struct s_static_array {
        tp_type elems_raw[tp_len] = {};

        constexpr s_static_array() = default;

        constexpr s_static_array(const std::initializer_list<tp_type> init) {
            size_t i = 0;

            for (const auto& elem : init) {
                elems_raw[i] = elem;
                ++i;
            }
        }

        tp_type& operator[](const size_t index) {
            assert(index < tp_len);
            return elems_raw[index];
        }

        const tp_type& operator[](const size_t index) const {
            assert(index < tp_len);
            return elems_raw[index];
        }

        operator s_array<tp_type>() {
            return {elems_raw, tp_len};
        }

        operator s_array<const tp_type>() const {
            return {elems_raw, tp_len};
        }
    };

    template<typename tp_type>
    struct s_list {
        tp_type* elems_raw = nullptr;
        size_t cap = 0;
        size_t len = 0;

        s_list() = default;

        s_list(tp_type* const elems, const size_t cap, const size_t len = 0) {
            assert(elems);
            assert(cap > 0);
            assert(len <= cap);

            elems_raw = elems;
            this->cap = cap;
            this->len = len;
        }

        tp_type& operator[](const size_t index) {
            assert(IsValid());
            assert(index < len);
            return elems_raw[index];
        }

        tp_type& operator[](const size_t index) const {
            assert(IsValid());
            assert(index < len);
            return elems_raw[index];
        }

        operator s_list<const tp_type>() const {
            return {elems_raw, len};
        }

        bool IsValid() const {
            return (!elems_raw && cap == 0 && len == 0) || (elems_raw && cap > 0 && len <= cap);
        }

        bool IsInitialized() const {
            assert(IsValid());
            return elems_raw;
        }

        bool Append(const tp_type& elem) {
            assert(IsValid());

            if (len == cap) {
                return false;
            }

            elems_raw[len] = elem;
            ++len;

            return true;
        }

        bool Append(tp_type&& elem) {
            assert(IsValid());

            if (len == cap) {
                return false;
            }

            elems_raw[len] = elem;
            ++len;

            return true;
        }

        void Pop() {
            assert(IsValid());
            assert(len > 0);

            --len;
        }

        tp_type& Last() const {
            assert(IsValid());
            assert(len > 0);
            return elems_raw[len - 1];
        }

        s_array<tp_type> ToArray() const {
            return {elems_raw, len};
        }
    };

    template<typename tp_type, size_t tp_cap>
    struct s_static_list {
        tp_type elems_raw[tp_cap] = {};
        size_t len = 0;

        constexpr s_static_list() = default;

        constexpr s_static_list(const std::initializer_list<tp_type> init) {
            size_t i = 0;

            for (const auto& elem : init) {
                assert(i < tp_cap);
                elems_raw[i] = elem;
                ++i;
            }

            len = i;
        }

        tp_type& operator[](const size_t index) {
            assert(index < len);
            return elems_raw[index];
        }

        const tp_type& operator[](const size_t index) const {
            assert(index < len);
            return elems_raw[index];
        }

        operator s_list<tp_type>() {
            return {elems_raw, tp_cap, len};
        }

        operator s_list<const tp_type>() const {
            return {elems_raw, tp_cap, len};
        }

        bool IsValid() const {
            return len <= tp_cap;
        }

        bool Append(const tp_type& elem) {
            assert(IsValid());

            if (len == tp_cap) {
                return false;
            }

            elems_raw[len] = elem;
            ++len;

            return true;
        }

        bool Append(tp_type&& elem) {
            assert(IsValid());

            if (len == tp_cap) {
                return false;
            }

            elems_raw[len] = elem;
            ++len;

            return true;
        }

        void Pop() {
            assert(IsValid());
            assert(len > 0);
            --len;
        }

        tp_type& Last() {
            assert(IsValid());
            assert(len > 0);
            return elems_raw[len - 1];
        }

        const tp_type& Last() const {
            assert(IsValid());
            assert(len > 0);
            return elems_raw[len - 1];
        }
    };

    struct s_mem_arena {
        void* buf = nullptr;
        size_t size = 0;
        size_t offs = 0;

        bool Init(const size_t size);
        void Clean();
        void* Push(const size_t size, const size_t alignment);

        bool IsValid() const {
            return (!buf && size == 0 && offs == 0) || (buf && size > 0 && offs <= size);
        }

        bool IsInitialized() const {
            assert(IsValid());
            return buf;
        }
    };

    bool IsTerminated(const s_array<const char> str_chrs);

    constexpr size_t BitsToBytes(const size_t bits) {
        return (bits + 7) & ~7;
    }

    constexpr size_t BytesToBits(const size_t bytes) {
        return bytes << 3;
    }

    constexpr size_t KilobytesToBytes(const size_t kb) {
        return kb * 1024;
    }

    constexpr size_t MegabytesToBytes(const size_t mb) {
        return mb * 1024 * 1024;
    }

    constexpr size_t GigabytesToBytes(const size_t gb) {
        return gb * 1024 * 1024 * 1024;
    }

    inline size_t ToIndex(const size_t x, const size_t y, const size_t width) {
        assert(x < width);
        return (y * width) + x;
    }

    constexpr bool IsPowerOfTwo(const size_t n) {
        return !(n & (n - 1));
    }

    inline size_t AlignForward(const size_t n, const size_t alignment) {
        assert(IsPowerOfTwo(alignment));
        return (n + alignment - 1) & ~(alignment - 1);
    }

    template <typename tp_type>
    bool IsDefault(const tp_type& obj) {
        const tp_type def = {};
        return memcmp(&obj, &def, sizeof(tp_type)) == 0;
    }

    template<typename tp_type>
    tp_type* PushType(s_mem_arena& mem_arena) {
        assert(mem_arena.IsInitialized());

        void* const ptr = mem_arena.Push(sizeof(tp_type), alignof(tp_type));

        if (!ptr) {
            return nullptr;
        }

        return new (ptr) tp_type();
    }

    template<typename tp_type>
    s_array<tp_type> PushArray(const size_t len, s_mem_arena& mem_arena) {
        assert(mem_arena.IsInitialized());

        const auto ptr = static_cast<tp_type*>(mem_arena.Push(sizeof(tp_type) * len, alignof(tp_type)));

        if (!ptr) {
            return {};
        }

        for (size_t i = 0; i < len; ++i) {
            new (&ptr[i]) tp_type();
        }

        return {ptr, len};
    }

    template<typename tp_type>
    s_list<tp_type> PushList(const size_t cap, s_mem_arena& mem_arena) {
        assert(mem_arena.IsInitialized());

        const auto ptr = static_cast<tp_type*>(mem_arena.Push(sizeof(tp_type) * cap, alignof(tp_type)));

        if (!ptr) {
            return {};
        }

        for (size_t i = 0; i < cap; ++i) {
            new (&ptr[i]) tp_type();
        }

        return {ptr, cap};
    }
}
