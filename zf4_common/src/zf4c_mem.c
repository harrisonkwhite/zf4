#include <zf4c_mem.h>

bool zf4_is_zero(const void* const data, const int size) {
    const ZF4Byte* const dataBytes = data;

    for (int i = 0; i < size; i++) {
        if (dataBytes[i]) {
            return false;
        }
    }

    return true;
}

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

bool zf4_init_mem_arena(ZF4MemArena* const arena, const int size) {
    assert(zf4_is_zero(arena, sizeof(*arena)));

    arena->buf = calloc(size, 1);

    if (!arena->buf) {
        return false;
    }

    arena->size = size;

    return true;
}

void zf4_clean_mem_arena(ZF4MemArena* const arena) {
    if (arena->buf) {
        free(arena->buf);
    }

    memset(arena, 0, sizeof(*arena));
}

void* zf4_push_to_mem_arena(ZF4MemArena* const arena, const int size, const int alignment) {
    assert(size > 0);
    assert(zf4_is_power_of_two(alignment));

    const int offsAligned = zf4_align_forward(arena->offs, alignment);
    const int offsNext = offsAligned + size;

    if (offsNext > arena->size) {
        return NULL;
    }

    arena->offs = offsNext;

    return (ZF4Byte*)arena->buf + offsAligned;
}

void zf4_reset_mem_arena(ZF4MemArena* const arena) {
    memset(arena->buf, 0, arena->offs);
    arena->offs = 0;
}
