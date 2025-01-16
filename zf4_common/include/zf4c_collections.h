#pragma once

#include <zf4c_mem.h>

namespace zf4 {
    template<SimpleType T, int Len>
    class StaticArray {
    public:
        static constexpr int get_len() {
            return Len;
        }

        T* get(const int index = 0) {
            assert(index >= 0 && index < Len);
            return &m_elems[index];
        }

        const T* get(const int index = 0) const {
            assert(index >= 0 && index < Len);
            return &m_elems[index];
        }

    private:
        T m_elems[Len];
    };

    template<SimpleType T>
    class Array {
    public:
        bool init(MemArena* const memArena, const int len);

        T* get(const int index = 0) {
            assert(index >= 0 && index < m_len);
            return &m_elems[index];
        }

        const T* get(const int index = 0) const {
            assert(index >= 0 && index < m_len);
            return &m_elems[index];
        }

        int get_len() const {
            return m_len;
        }

    private:
        SafePtr<T> m_elems;
        int m_len;
    };

    template<SimpleType T, int Cap>
    class StaticStack {
    public:
        static constexpr int get_cap() {
            return Cap;
        }

        T* get(const int index = 0) {
            assert(index < m_len);
            return m_array.get(index);
        }

        const T* get(const int index = 0) const {
            assert(index < m_len);
            return m_array.get(index);
        }

        int get_len() const {
            return m_len;
        }

        bool is_empty() const {
            return m_len == 0;
        }

        bool is_full() const {
            return m_len == get_cap();
        }

        bool push(const T& elem) {
            if (is_full()) {
                return false;
            }

            *m_array.get(m_len) = elem;
            ++m_len;

            return true;
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

    private:
        StaticArray<T, Cap> m_array;
        int m_len;
    };

    template<SimpleType T>
    class Stack {
    public:
        bool init(MemArena* const memArena, const int cap);

        T* get(const int index = 0) {
            assert(index < m_len);
            return m_array.get(index);
        }

        const T* get(const int index = 0) const {
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

        bool push(const T& elem) {
            if (is_full()) {
                return false;
            }

            *m_array.get(m_len) = elem;
            ++m_len;

            return true;
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

    template<SimpleType T, int Len>
    class StaticActivityArray {
    public:
        static constexpr int get_len() {
            return Len;
        }

        T* get(const int index = 0) {
            assert(is_active(index));
            return m_array.get(index);
        }

        const T* get(const int index = 0) const {
            assert(is_active(index));
            return m_array.get(index);
        }

        void activate(const int index) {
            assert(index >= 0 && index < Len);
            activate_bit(m_activity, index);
        }

        void deactivate(const int index) {
            assert(index >= 0 && index < Len);
            deactivate_bit(m_activity, index);
        }

        bool is_active(const int index) const {
            return is_bit_active(m_activity, index);
        }

        int get_first_active_index() const {
            return get_first_active_bit_index(m_activity, Len);
        }

        int get_first_inactive_index() const {
            return get_first_inactive_bit_index(m_activity, Len);
        }

    private:
        StaticArray<T, Len> m_array;
        Byte m_activity[bits_to_bytes(Len)];
    };

    template<SimpleType T>
    class ActivityArray {
    public:
        bool init(MemArena* const memArena, const int len);

        T* get(const int index = 0) {
            assert(is_active(index));
            return m_array.get(index);
        }

        const T* get(const int index = 0) const {
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
        SafePtr<Byte> m_activity;
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
