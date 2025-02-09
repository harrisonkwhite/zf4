#pragma once

#include <cstdio>
#include <zf4c_mem.h>

namespace zf4 {
    void Log(const char* const format, ...);
    void LogError(const char* const format, ...);

    char* GetFileContentsStr(const char* const filename, int* const len);

    template<c_trivial_type tp_type>
    inline tp_type& ReadFromFS(FILE* const fs, const int cnt = 1) {
        assert(fs);
        assert(cnt >= 1);

        tp_type val;
        fread(&val, sizeof(val), cnt, fs);
        return val;
    }
}
