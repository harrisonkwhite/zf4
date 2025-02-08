#include <zf4c_io.h>

#include <cstdio>
#include <cstdarg>
#include <zf4c_mem.h>

namespace zf4 {
    void Log(const char* const format, ...) {
        va_list args;
        va_start(args, format);
        std::vprintf(format, args);
        std::printf("\n");
        va_end(args);
    }

    void LogError(const char* const format, ...) {
        va_list args;
        va_start(args, format);

        std::fprintf(stderr, "ERROR: ");
        std::vfprintf(stderr, format, args);
        std::fprintf(stderr, "\n");

        va_end(args);
    }

    char* GetFileContentsStr(const char* const filename, int* const len) {
        assert(filename);

        // Open the file.
        FILE* const fs = std::fopen(filename, "rb");

        if (!fs) {
            return nullptr;
        }

        // Get the file size.
        std::fseek(fs, 0, SEEK_END);
        const long file_size = std::ftell(fs);
        std::fseek(fs, 0, SEEK_SET);

        // Allocate memory to store the file contents.
        const int contents_len = file_size;
        const int contents_size = contents_len + 1; // Account for the '\0'.
        const auto contents = static_cast<char*>(std::calloc(contents_size, 1));

        if (!contents) {
            std::fclose(fs);
            return nullptr;
        }

        // Read the contents into the buffer.
        std::fread(contents, 1, contents_size, fs);
        contents[contents_size - 1] = '\0';

        std::fclose(fs);

        // Update the length variable if provided.
        if (len) {
            *len = contents_len;
        }

        return contents;
    }
}
