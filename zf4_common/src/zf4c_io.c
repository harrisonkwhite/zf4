#include <zf4c_io.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

void zf4_log(const char* const format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    printf("\n");
    va_end(args);
}

void zf4_log_error(const char* const format, ...) {
    va_list args;
    va_start(args, format);

    fprintf(stderr, "ERROR: ");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");

    va_end(args);
}

char* zf4_get_file_contents(const char* const filename) {
    // Open the file.
    FILE* const fs = fopen(filename, "rb");

    if (!fs) {
        return NULL;
    }

    // Get the file size.
    fseek(fs, 0, SEEK_END);
    const long fileSize = ftell(fs);
    fseek(fs, 0, SEEK_SET);

    // Allocate memory to store the file contents.
    const int contentsSize = fileSize + 1; // Accounts for '\0'.
    char* const contents = malloc(contentsSize);

    if (!contents) {
        fclose(fs);
        return NULL;
    }

    // Read the contents into the buffer.
    fread(contents, 1, contentsSize - 1, fs);
    contents[contentsSize - 1] = '\0';

    fclose(fs);

    return contents;
}
