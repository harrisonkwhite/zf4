#include <zf4c_mem.h>

namespace zf4 {
    int get_first_active_bit_index(const Byte* const bytes, const int bitCnt) {
        for (int i = 0; i < bitCnt; ++i) {
            if (is_bit_active(bytes, i)) {
                return i;
            }
        }

        return -1;
    }

    int get_first_inactive_bit_index(const Byte* const bytes, const int bitCnt) {
        for (int i = 0; i < bitCnt; ++i) {
            if (!is_bit_active(bytes, i)) {
                return i;
            }
        }

        return -1;
    }

    bool are_all_bits_active(const Byte* const bytes, const int bitCnt) {
        return get_first_inactive_bit_index(bytes, bitCnt) == -1;
    }

    bool are_all_bits_inactive(const Byte* const bytes, const int bitCnt) {
        return get_first_active_bit_index(bytes, bitCnt) == -1;
    }

    bool MemArena::init(const int size) {
        assert(is_zero(this));

        m_bytes = alloc_zeroed<Byte>(size);

        if (!m_bytes) {
            return false;
        }

        m_size = size;

        return true;
    }

    void MemArena::clean() {
        if (m_bytes) {
            std::free(m_bytes);
        }

        zero_out(this);
    }

    SafePtr<Byte> MemArena::alloc(const int size, const int alignment) {
        assert(m_bytes);
        assert(size > 0);
        assert(is_power_of_two(alignment));

        const int offsAligned = align_forward(m_offs, alignment);
        const int offsNext = offsAligned + size;

        if (offsNext > m_size) {
            return {};
        }

        m_offs = offsNext;

        return {m_bytes + offsAligned, size};
    }

    void MemArena::reset() {
        assert(m_bytes);

        std::memset(m_bytes, 0, m_offs);
        m_offs = 0;
    }
}
