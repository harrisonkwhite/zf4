#ifndef ZF4C_MEM_H
#define ZF4C_MEM_H

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define ZF4_KILOBYTES(X) (X * 1024)
#define ZF4_MEGABYTES(X) (X * 1024 * 1024)
#define ZF4_GIGABYTES(X) (X * 1024 * 1024 * 1024)

typedef unsigned char ZF4Byte;

typedef struct {
    void* buf;
    int size;
    int offs;
} ZF4MemArena;

bool zf4_is_power_of_two(const int n);
int zf4_align_forward(const int n, const int alignment);
bool zf4_is_zero(const void* const data, const int size);

bool zf4_init_mem_arena(ZF4MemArena* const arena, const int size);
void zf4_clean_mem_arena(ZF4MemArena* const arena);
void* zf4_push_to_mem_arena(ZF4MemArena* const arena, const int size, const int alignment);
void zf4_reset_mem_arena(ZF4MemArena* const arena);

#endif
