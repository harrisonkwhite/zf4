#include <zf4c_mem.h>

int zf4_get_first_active_bit_index(const ZF4Byte* const bytes, const int bitCnt) {
    for (int i = 0; i < bitCnt; ++i) {
        if (zf4_is_bit_active(bytes, i)) {
            return i;
        }
    }

    return -1;
}

int zf4_get_first_inactive_bit_index(const ZF4Byte* const bytes, const int bitCnt) {
    for (int i = 0; i < bitCnt; ++i) {
        if (!zf4_is_bit_active(bytes, i)) {
            return i;
        }
    }

    return -1;
}

bool zf4_are_all_bits_active(const ZF4Byte* const bytes, const int bitCnt) {
    return zf4_get_first_inactive_bit_index(bytes, bitCnt) == -1;
}

bool zf4_are_all_bits_inactive(const ZF4Byte* const bytes, const int bitCnt) {
    return zf4_get_first_active_bit_index(bytes, bitCnt) == -1;
}

bool ZF4MemArena::init(const int size) {
    assert(zf4_is_zero(this));

    bytes = zf4_alloc_zeroed<ZF4Byte>(size);

    if (!bytes) {
        return false;
    }

    this->size = size;

    return true;
}

void ZF4MemArena::clean() {
    if (bytes) {
        std::free(bytes);
    }

    zf4_zero_out(this);
}

ZF4Byte* ZF4MemArena::push(const int size, const int alignment) {
    assert(bytes);
    assert(size > 0);
    assert(zf4_is_power_of_two(alignment));

    const int offsAligned = zf4_align_forward(offs, alignment);
    const int offsNext = offsAligned + size;

    if (offsNext > this->size) {
        return nullptr;
    }

    offs = offsNext;

    return bytes + offsAligned;
}

void ZF4MemArena::reset() {
    assert(bytes);

    std::memset(bytes, 0, offs);
    offs = 0;
}
