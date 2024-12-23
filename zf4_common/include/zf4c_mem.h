#ifndef ZF4C_MEM_H
#define ZF4C_MEM_H

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define ZF4_KILOBYTES(X) (X * 1024)
#define ZF4_MEGABYTES(X) (X * 1024 * 1024)
#define ZF4_GIGABYTES(X) (X * 1024 * 1024 * 1024)
#define ZF4_BITS_TO_BYTES(X) ((X + 7) & ~7)

typedef unsigned char ZF4Byte;

typedef struct {
    void* buf;
    int size;
    int offs;
} ZF4MemArena;

bool zf4_is_zero(const void* const data, const int size);

int zf4_get_first_active_bit_index(const ZF4Byte* const bytes, const int bitCnt);
int zf4_get_first_inactive_bit_index(const ZF4Byte* const bytes, const int bitCnt);
bool zf4_are_all_bits_active(const ZF4Byte* const bytes, const int bitCnt);
bool zf4_are_all_bits_inactive(const ZF4Byte* const bytes, const int bitCnt);

bool zf4_init_mem_arena(ZF4MemArena* const arena, const int size);
void zf4_clean_mem_arena(ZF4MemArena* const arena);
void* zf4_push_to_mem_arena(ZF4MemArena* const arena, const int size, const int alignment);
void zf4_reset_mem_arena(ZF4MemArena* const arena);

inline bool zf4_is_power_of_two(const int n) {
    return !(n & (n - 1));
}

inline int zf4_align_forward(const int n, const int alignment) {
    assert(n >= 0);
    assert(zf4_is_power_of_two(alignment));
    return (n + alignment - 1) & ~(alignment - 1);
}

inline void zf4_activate_bit(ZF4Byte* const bytes, const int bitIndex) {
    bytes[bitIndex / 8] |= 1 << (bitIndex % 8);
}

inline void zf4_deactivate_bit(ZF4Byte* const bytes, const int bitIndex) {
    bytes[bitIndex / 8] &= ~(1 << (bitIndex % 8));
}

inline void zf4_clear_bits(ZF4Byte* const bytes, const int bitCnt) {
    memset(bytes, 0, ZF4_BITS_TO_BYTES(bitCnt));
}

inline bool zf4_is_bit_active(const ZF4Byte* const bytes, const int bitIndex) {
    return bytes[bitIndex / 8] & (1 << (bitIndex % 8));
}

#endif
