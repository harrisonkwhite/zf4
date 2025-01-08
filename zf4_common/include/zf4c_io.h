#pragma once

#include <cstdio>
#include <zf4c_mem.h>

namespace zf4 {
    void log(const char* const format, ...);
    void log_error(const char* const format, ...);

    char* get_file_contents(const char* const filename, int* const len = nullptr);

    template<SimpleType T>
    int read_from_fs(T* const data, FILE* const fs, const int cnt = 1) {
        return std::fread(data, sizeof(*data), cnt, fs);
    }
}
