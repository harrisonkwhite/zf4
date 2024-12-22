#include <zf4c_logging.h>

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
