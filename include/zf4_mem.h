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
    public:
        s_array() = default;

        s_array(tp_type* const elems, const int len) {
            assert(elems && len > 0);

            m_elems = elems;
            m_len = len;
        }

        tp_type& operator[](const int index) {
            assert(IsInitialized());
            assert(index >= 0 && index < m_len);
            return m_elems[index];
        }

        tp_type& operator[](const int index) const {
            assert(IsInitialized());
            assert(index >= 0 && index < m_len);
            return m_elems[index];
        }

        operator s_array<const tp_type>() const {
            return {m_elems, m_len};
        }

        bool IsInitialized() const {
            return m_elems;
        }

        tp_type* Raw() const {
            assert(IsInitialized());
            return m_elems;
        }

        int Len() const {
            assert(IsInitialized());
            return m_len;
        }

        int SizeInBytes() const {
            assert(IsInitialized());
            return m_len * sizeof(tp_type);
        }

    private:
        tp_type* m_elems = nullptr;
        int m_len = 0;
    };

    template<typename tp_type, int tp_len>
    struct s_static_array {
        constexpr s_static_array() = default;

        constexpr s_static_array(const std::initializer_list<tp_type> init) {
            int i = 0;

            for (const auto& elem : init) {
                m_elems[i] = elem;
                ++i;
            }
        }

        tp_type& operator[](const int index) {
            assert(index >= 0 && index < tp_len);
            return m_elems[index];
        }

        const tp_type& operator[](const int index) const {
            assert(index >= 0 && index < tp_len);
            return m_elems[index];
        }

        operator s_array<tp_type>() {
            return {m_elems, tp_len};
        }

        operator s_array<const tp_type>() const {
            return {m_elems, tp_len};
        }

    private:
        tp_type m_elems[tp_len] = {};
    };

    template<typename tp_type>
    struct s_list {
    public:
        s_list() = default;

        s_list(tp_type* const elems, const int cap) {
            assert(elems && cap > 0);

            m_elems = elems;
            m_cap = cap;
        }

        bool Append(tp_type&& elem);

        tp_type& operator[](const int index) {
            assert(IsInitialized());
            assert(index >= 0 && index < m_len);
            return m_elems[index];
        }

        tp_type& operator[](const int index) const {
            assert(IsInitialized());
            assert(index >= 0 && index < m_len);
            return m_elems[index];
        }

        operator s_list<const tp_type>() const {
            return {m_elems, m_len};
        }

        bool IsInitialized() const {
            return m_elems;
        }

        tp_type* Raw() const {
            assert(IsInitialized());
            return m_elems;
        }

        int Cap() const {
            assert(IsInitialized());
            return m_cap;
        }

        int Len() const {
            assert(IsInitialized());
            return m_len;
        }

    private:
        tp_type* m_elems = nullptr;
        int m_cap = 0;
        int m_len = 0;
    };

    struct s_mem_arena {
    public:
        bool Init(const int size);
        void Clean();
        void Rewind(const int offs);

        bool IsInitialized() const {
            return m_buf;
        }

        int Offset() const {
            return m_offs;
        }

        template<typename tp_type>
        tp_type* Push() {
            assert(IsInitialized());

            void* const ptr = Push(sizeof(tp_type), alignof(tp_type));

            if (!ptr) {
                return nullptr;
            }

            return new (ptr) tp_type();
        }

        template<typename tp_type>
        s_array<tp_type> PushArray(const int len) {
            assert(IsInitialized());
            assert(len > 0);

            const auto ptr = static_cast<tp_type*>(Push(sizeof(tp_type) * len, alignof(tp_type)));

            if (!ptr) {
                return {};
            }

            for (int i = 0; i < len; ++i) {
                new (&ptr[i]) tp_type();
            }

            return {ptr, len};
        }

    private:
        void* m_buf = nullptr;
        int m_size = 0;
        int m_offs = 0;

        void* Push(const int size, const int alignment);
    };

    bool IsTerminated(const s_array<const char> str_chrs);

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

    template<typename tp_type>
    inline bool s_list<tp_type>::Append(tp_type&& elem) {
        assert(IsInitialized());

        if (m_len == m_cap) {
            return false;
        }

        m_elems[m_len] = elem;
        ++m_len;

        return true;
    }
}
