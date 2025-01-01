#ifndef ZF4C_IO_H
#define ZF4C_IO_H

#include <cstdio>
#include <zf4c_mem.h>

void zf4_log(const char* const format, ...);
void zf4_log_error(const char* const format, ...);

char* zf4_get_file_contents(const char* const filename);

template<ZF4SimpleType T>
int zf4_read_from_fs(T* const data, FILE* const fs, const int cnt = 1) {
    return std::fread(data, sizeof(*data), cnt, fs);
}

#endif
