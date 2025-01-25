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
    void* buf;
    int size;
    int offs;
} s_mem_arena;

void Clear(void* const buf, const int size);
bool IsClear(const void* const buf, const int size);

int ActiveBitCnt(const ta_byte* const bytes, const int bit_cnt);
int InactiveBitCnt(const ta_byte* const bytes, const int bit_cnt);
int IndexOfFirstActiveBit(const ta_byte* const bytes, const int bit_cnt);
int IndexOfFirstInactiveBit(const ta_byte* const bytes, const int bit_cnt);
bool AreAllBitsActive(const ta_byte* const bytes, const int bit_cnt);
bool AreAllBitsInactive(const ta_byte* const bytes, const int bit_cnt);

bool InitMemArena(s_mem_arena* const mem_arena, const int size);
void CleanMemArena(s_mem_arena* const mem_arena);
void EmptyMemArena(s_mem_arena* const mem_arena);
void RewindMemArena(s_mem_arena* const mem_arena, const int offs);

void* Push(const int size, s_mem_arena* const mem_arena);
void* PushAligned(const int size, const int alignment, s_mem_arena* const mem_arena);

inline int AlignForward(const int n, const int alignment) {
    assert(n >= 0);
    assert(IS_POWER_OF_TWO(alignment));
    return (n + alignment - 1) & ~(alignment - 1);
}

inline void ActivateBit(const int bit_index, ta_byte* const bytes, const int bit_cnt) {
    assert(bit_index >= 0 && bit_index < bit_cnt);
    assert(bytes);
    bytes[bit_index / 8] |= 1 << (bit_index % 8);
}

inline void DeactivateBit(const int bit_index, ta_byte* const bytes, const int bit_cnt) {
    assert(bit_index >= 0 && bit_index < bit_cnt);
    assert(bytes);
    bytes[bit_index / 8] &= ~(1 << (bit_index % 8));
}

inline bool IsBitActive(const int bit_index, ta_byte* const bytes, const int bit_cnt) {
    assert(bit_index >= 0 && bit_index < bit_cnt);
    assert(bytes);
    return bytes[bit_index / 8] & (1 << (bit_index % 8));
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
