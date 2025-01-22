#ifndef ZF4C_MEM_H
#define ZF4C_MEM_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdalign.h>

#define BITS_TO_BYTES(X) ((X + 7) & ~7)
#define BYTES_TO_BITS(X) (X << 3)
#define KILOBYTES_TO_BYTES(X) (X * 1024)
#define MEGABYTES_TO_BYTES(X) (X * 1024 * 1024)
#define GIGABYTES_TO_BYTES(X) (X * 1024 * 1024 * 1024)
#define IS_POWER_OF_TWO(X) (!(X & (X - 1)))

extern const int g_first_active_bit_indexes[256]; // For mapping a byte to the index of its first active bit.
extern const int g_first_inactive_bit_indexes[256]; // For mapping a byte to the index of its first inactive bit.
extern const int g_active_bit_cnts[256]; // For mapping a byte to the number of active bits it contains.

typedef uint8_t ta_byte;

typedef struct {
    ta_byte* bytes;
    int bit_cnt;
} s_bitset;

typedef struct {
    const ta_byte* bytes;
    int bit_cnt;
} s_bitset_view;

typedef struct {
    void* buf;
    int size;
    int offs;
} s_mem_arena;

void Clear(void* const buf, const int size);
bool IsClear(const void* const buf, const int size);

int ActiveBitCnt(const s_bitset_view* const bitset);
int InactiveBitCnt(const s_bitset_view* const bitset);
int IndexOfFirstActiveBit(const s_bitset_view* const bitset);
int IndexOfFirstInactiveBit(const s_bitset_view* const bitset);
bool AreAllBitsActive(const s_bitset_view* const bitset);
bool AreAllBitsInactive(const s_bitset_view* const bitset);

bool InitMemArena(s_mem_arena* const mem_arena, const int size);
void CleanMemArena(s_mem_arena* const mem_arena);
void ResetMemArena(s_mem_arena* const mem_arena);
void* Push(const int size, s_mem_arena* const mem_arena);
void* PushAligned(const int size, const int alignment, s_mem_arena* const mem_arena);

inline int AlignForward(const int n, const int alignment) {
    assert(n >= 0);
    assert(IsPowerOfTwo(alignment));
    return (n + alignment - 1) & ~(alignment - 1);
}

inline bool IsBitsetValid(const s_bitset* const bitset) {
    assert(bitset);

    if (IsClear(bitset, sizeof(*bitset))) {
        return true;
    }

    return bitset->bytes && bitset->bit_cnt > 0;
}

inline void ActivateBit(s_bitset* const bitset, const int bit_index) {
    assert(bit_index >= 0 && bit_index < bitset->bit_cnt);
    bitset->bytes[bit_index / 8] |= 1 << (bit_index % 8);
}

inline void DeactivateBit(s_bitset* const bitset, const int bit_index) {
    assert(bit_index >= 0 && bit_index < bitset->bit_cnt);
    bitset->bytes[bit_index / 8] &= ~(1 << (bit_index % 8));
}

inline bool IsBitActive(const s_bitset_view* const bitset, const int bit_index) {
    assert(bit_index >= 0 && bit_index < bitset->bit_cnt);
    return bitset->bytes[bit_index / 8] & (1 << (bit_index % 8));
}

inline bool IsMemArenaValid(const s_mem_arena* const mem_arena) {
    assert(mem_arena);

    if (IsClear(mem_arena, sizeof(*mem_arena))) {
        return true;
    }

    return mem_arena->buf
        && mem_arena->size > 0
        && mem_arena->offs >= 0 && mem_arena->offs <= mem_arena->size;
}

#endif
