#include <zf4c_mem.h>

bool zf4_is_power_of_two(const int n) {
    return !(n & (n - 1));
}

int zf4_align_forward(const int n, const int alignment) {
    assert(n >= 0);
    assert(zf4_is_power_of_two(alignment));
    return (n + alignment - 1) & ~(alignment - 1);
}

bool zf4_is_zero(const void* const data, const int size) {
    const ZF4Byte* const dataBytes = data;

    for (int i = 0; i < size; i++) {
        if (dataBytes[i]) {
            return false;
        }
    }

    return true;
}

bool zf4_init_mem_arena(ZF4MemArena* const arena, const int size) {
    assert(zf4_is_zero(arena, sizeof(*arena)));

    arena->buf = malloc(size);

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
