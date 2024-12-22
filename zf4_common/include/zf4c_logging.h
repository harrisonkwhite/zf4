#ifndef ZF4C_LOGGING_H
#define ZF4C_LOGGING_H

#include <stdio.h>
#include <stdarg.h>

void zf4_log(const char* const format, ...);
void zf4_log_error(const char* const format, ...);

#endif
