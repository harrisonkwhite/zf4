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

        bytes = alloc_zeroed<Byte>(size);

        if (!bytes) {
            return false;
        }

        this->size = size;

        return true;
    }

    void MemArena::clean() {
        if (bytes) {
            std::free(bytes);
        }

        zero_out(this);
    }

    Byte* MemArena::push(const int size, const int alignment) {
        assert(bytes);
        assert(size > 0);
        assert(is_power_of_two(alignment));

        const int offsAligned = align_forward(offs, alignment);
        const int offsNext = offsAligned + size;

        if (offsNext > this->size) {
            return nullptr;
        }

        offs = offsNext;

        return bytes + offsAligned;
    }

    void MemArena::reset() {
        assert(bytes);

        std::memset(bytes, 0, offs);
        offs = 0;
    }
}
