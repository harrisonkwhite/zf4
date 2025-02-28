#pragma once

#include <cstdio>
#include <zf4_mem.h>

namespace zf4 {
    void Log(const char* const format, ...);
    void LogError(const char* const format, ...);
    s_array<const a_byte> PushFileContents(const char* const filename, s_mem_arena& mem_arena, bool& error);

    template<c_trivial_type tp_type>
    inline tp_type ReadFromFS(FILE* const fs) {
        assert(fs);

        tp_type val;
        fread(&val, sizeof(val), 1, fs);
        return val;
    }
}
