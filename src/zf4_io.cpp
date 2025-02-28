#include <zf4_io.h>

#include <cstdarg>

namespace zf4 {
    void Log(const char* const format, ...) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        printf("\n");
        va_end(args);
    }

    void LogError(const char* const format, ...) {
        va_list args;
        va_start(args, format);

        fprintf(stderr, "ERROR: ");
        vfprintf(stderr, format, args);
        fprintf(stderr, "\n");

        va_end(args);
    }

    s_array<const a_byte> PushFileContents(const char* const filename, s_mem_arena& mem_arena, bool& error) {
        assert(!error);

        FILE* const fs = fopen(filename, "rb");

        if (!fs) {
            error = true;
            return {};
        }

        fseek(fs, 0, SEEK_END);
        const size_t size = ftell(fs);
        fseek(fs, 0, SEEK_SET);

        const auto contents = PushArray<a_byte>(size, mem_arena);

        if (IsStructZero(contents)) {
            fclose(fs);
            error = true;
            return {};
        }

        fread(contents.elems_raw, sizeof(a_byte), contents.len, fs);

        fclose(fs);

        return contents;
    }

    size_t FileSize(FILE* const fs) {
        const size_t pos_init = ftell(fs);
        fseek(fs, 0, SEEK_END);
        const size_t size = ftell(fs);
        fseek(fs, pos_init, SEEK_SET);
        return size;
    }
}
