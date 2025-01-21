#ifndef ZF4C_IO_H
#define ZF4C_IO_H

void Log(const char* const format, ...);
void LogError(const char* const format, ...);

char* GetFileContents(const char* const filename, int* const len);

#endif
