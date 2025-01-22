#include <zf4c_mem.h>

const int g_first_active_bit_indexes[256] = {
    -1, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
};

const int g_first_inactive_bit_indexes[256] = {
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 7,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 8
};

const int g_active_bit_cnts[256] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};

void Clear(void* const buf, const int size) {
    assert(buf);
    assert(size > 0);

    memset(buf, 0, size);
}

bool IsClear(const void* const buf, const int size) {
    assert(buf);
    assert(size > 0);

    const ta_byte* const bytes = (const ta_byte*)buf;

    for (int i = 0; i < size; ++i) {
        if (bytes[i]) {
            return false;
        }
    }

    return true;
}

int ActiveBitCnt(const ta_byte* const bytes, const int bit_cnt) {
    assert(bytes);
    assert(bit_cnt > 0);

    int cnt = 0;

    for (int i = 0; i < BITS_TO_BYTES(bit_cnt); ++i) {
        cnt += g_active_bit_cnts[bytes[i]];
    }

    return cnt;
}

int InactiveBitCnt(const ta_byte* const bytes, const int bit_cnt) {
    assert(bytes);
    assert(bit_cnt > 0);
    return bit_cnt - ActiveBitCnt(bytes, bit_cnt);
}

int IndexOfFirstActiveBit(const ta_byte* const bytes, const int bit_cnt) {
    assert(bytes);
    assert(bit_cnt > 0);

    for (int i = 0; i < BITS_TO_BYTES(bit_cnt); ++i) {
        if (bytes[i]) {
            return (i * 8) + g_first_active_bit_indexes[bytes[i]];
        }
    }

    return -1;
}

int IndexOfFirstInactiveBit(const ta_byte* const bytes, const int bit_cnt) {
assert(bytes);
    assert(bit_cnt > 0);

    for (int i = 0; i < BITS_TO_BYTES(bit_cnt); ++i) {
        if (bytes[i] != 0xFF) {
            return (i * 8) + g_first_inactive_bit_indexes[bytes[i]];
        }
    }

    return -1;
}

bool AreAllBitsActive(const ta_byte* const bytes, const int bit_cnt) {
assert(bytes);
    assert(bit_cnt > 0);

    for (int i = 0; i < BITS_TO_BYTES(bit_cnt); ++i) {
        if (bytes[i] != 0xFF) {
            return false;
        }
    }

    return true;
}

bool AreAllBitsInactive(const ta_byte* const bytes, const int bit_cnt) {
assert(bytes);
    assert(bit_cnt > 0);

    for (int i = 0; i < BITS_TO_BYTES(bit_cnt); ++i) {
        if (bytes[i]) {
            return false;
        }
    }

    return true;
}

bool InitMemArena(s_mem_arena* const mem_arena, const int size) {
    assert(mem_arena);
    assert(IsClear(mem_arena, sizeof(*mem_arena)));
    assert(size > 0);

    mem_arena->buf = calloc(1, size);

    if (!mem_arena->buf) {
        return false;
    }

    mem_arena->size = size;

    return true;
}

void CleanMemArena(s_mem_arena* const mem_arena) {
    assert(mem_arena);

    if (mem_arena->buf) {
        free(mem_arena->buf);
    }

    Clear(mem_arena, sizeof(*mem_arena));
}

void ResetMemArena(s_mem_arena* const mem_arena) {
    assert(mem_arena);

    if (mem_arena->offs > 0) {
        Clear(mem_arena->buf, mem_arena->offs);
    }

    mem_arena->offs = 0;
}

void* Push(const int size, s_mem_arena* const mem_arena) {
    assert(size > 0);
    assert(mem_arena);

    const int offs_next = mem_arena->offs + size;

    if (offs_next > mem_arena->size) {
        assert(false && "Failed to push to memory arena due to insufficient space!");
        return NULL;
    }

    void* const ptr = (char*)mem_arena->buf + mem_arena->offs;

    mem_arena->offs = offs_next;

    return ptr;
}

void* PushAligned(const int size, const int alignment, s_mem_arena* const mem_arena) {
    assert(mem_arena);

    const int offs_aligned = AlignForward(mem_arena->offs, alignment);
    const int offs_next = offs_aligned + size;

    if (offs_next > mem_arena->size) {
        assert(false && "Failed to push to memory arena due to insufficient space!");
        return NULL;
    }

    mem_arena->offs = offs_next;

    return (char*)mem_arena->buf + offs_aligned;
}
