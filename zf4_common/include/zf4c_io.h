#pragma once

namespace zf4 {
    void Log(const char* const format, ...);
    void LogError(const char* const format, ...);

    char* GetFileContents(const char* const filename, int* const len);
}
