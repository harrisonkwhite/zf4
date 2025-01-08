#include <zf4c_io.h>

#include <cstdarg>
#include <zf4c_mem.h>

namespace zf4 {
    void log(const char* const format, ...) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        printf("\n");
        va_end(args);
    }

    void log_error(const char* const format, ...) {
        va_list args;
        va_start(args, format);

        fprintf(stderr, "ERROR: ");
        vfprintf(stderr, format, args);
        fprintf(stderr, "\n");

        va_end(args);
    }

    char* get_file_contents(const char* const filename, int* const len) {
        // Open the file.
        FILE* const fs = fopen(filename, "rb");

        if (!fs) {
            return nullptr;
        }

        // Get the file size.
        fseek(fs, 0, SEEK_END);
        const long fileSize = ftell(fs);
        fseek(fs, 0, SEEK_SET);

        // Allocate memory to store the file contents.
        const int contentsLen = fileSize;
        const int contentsSize = contentsLen + 1; // Account for the '\0'.
        const auto contents = alloc<char>(contentsSize);

        if (!contents) {
            fclose(fs);
            return nullptr;
        }

        // Read the contents into the buffer.
        read_from_fs<char>(contents, fs, contentsSize - 1);
        contents[contentsSize - 1] = '\0';

        fclose(fs);

        // Update the length variable if provided.
        if (len) {
            *len = contentsLen;
        }

        return contents;
    }
}
