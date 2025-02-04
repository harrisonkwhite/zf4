#pragma once

#include <zf4c_math.h>

namespace zf4 {
    void InitRNG(void);
    int RandInt(const int min, const int max);
    float RandFloat(const float min, const float max);
    float RandPerc(void);

    inline float RandDir(void) {
        return RandFloat(0.0f, 2.0f * g_pi);
    }
}
