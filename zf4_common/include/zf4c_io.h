#pragma once

namespace zf4 {
    void Log(const char* const format, ...);
    void LogError(const char* const format, ...);

    char* GetFileContentsStr(const char* const filename, int* const len);
}
