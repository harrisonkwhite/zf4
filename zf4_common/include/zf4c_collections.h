#pragma once

#include <zf4c_mem.h>

namespace zf4 {
    template<SimpleType T>
    class Array {
    public:
        bool init(MemArena* const memArena, const int len);

        T* get(const int index) {
            assert(index >= 0 && index < m_len);
            return &m_elems[index];
        }

        const T* get(const int index) const {
            assert(index >= 0 && index < m_len);
            return &m_elems[index];
        }

        int get_len() const {
            return m_len;
        }

    private:
        MemArenaAlloc<T> m_elems;
        int m_len;
    };

    template<SimpleType T>
    class Stack {
    public:
        bool init(MemArena* const memArena, const int cap);

        T* get(const int index) {
            assert(index < m_len);
            return m_array.get(index);
        }

        const T* get(const int index) const {
            assert(index < m_len);
            return m_array.get(index);
        }

        int get_len() const {
            return m_len;
        }

        int get_capacity() const {
            return m_array.get_len();
        }

        bool is_empty() const {
            return m_len == 0;
        }

        bool is_full() const {
            return m_len == get_capacity();
        }

        void push(const T& elem) {
            *m_array.get(m_len) = elem;
            ++m_len;
        }

        void pop() {
            assert(m_len > 0);
            --m_len;
        }

        T* peek() {
            return m_array.get(m_len - 1);
        }

        const T* peek() const {
            return m_array.get(m_len - 1);
        }

        void clear() {
            m_len = 0;
        }

        Array<T> to_array() {
            return m_array;
        }

    private:
        Array<T> m_array;
        int m_len;
    };

    // A form of array where each element has an "activity" state associated with it, indicated by a bit in a bitset.
    template<SimpleType T>
    class ActivityArray {
    public:
        bool init(MemArena* const memArena, const int len);

        T* get(const int index) {
            assert(is_active(index));
            return m_array.get(index);
        }

        const T* get(const int index) const {
            assert(is_active(index));
            return m_array.get(index);
        }

        int get_len() const {
            return m_array.get_len();
        }

        bool is_active(const int index) const {
            return is_bit_active(m_activity.get(), index);
        }

        void activate(const int index) {
            assert(index >= 0 && index < get_len());
            activate_bit(m_activity.get(), index);
        }

        void deactivate(const int index) {
            assert(index >= 0 && index < get_len());
            deactivate_bit(m_activity.get(), index);
        }

        int get_first_active_index() const {
            return get_first_active_bit_index(m_activity.get(), get_len());
        }

        int get_first_inactive_index() const {
            return get_first_inactive_bit_index(m_activity.get(), get_len());
        }

        Array<T> to_array() {
            return m_array;
        }

    private:
        Array<T> m_array;
        MemArenaAlloc<Byte> m_activity;
    };

    template<SimpleType T>
    inline bool Array<T>::init(MemArena* const memArena, const int len) {
        assert(is_zero(this));
        assert(len > 0);

        m_elems = memArena->alloc<T>(len);

        if (!m_elems) {
            return false;
        }

        m_len = len;

        return true;
    }

    template<SimpleType T>
    inline bool Stack<T>::init(MemArena* const memArena, const int cap) {
        assert(is_zero(this));

        if (!m_array.init(memArena, cap)) {
            return false;
        }

        return true;
    }

    template<SimpleType T>
    inline bool ActivityArray<T>::init(MemArena* const memArena, const int len) {
        assert(is_zero(this));

        if (!m_array.init(memArena, len)) {
            return false;
        }

        m_activity = memArena->alloc<Byte>(bits_to_bytes(len));

        if (!m_activity) {
            zero_out(this);
            return false;
        }

        return true;
    }
}
