#pragma once

#include <zf4c_math.h>

namespace zf4 {
    void InitRNG(void);
    int RandInt(const int min, const int max);
    float RandFloat(const float min, const float max);
    float RandPerc(void);

    inline int RandInt(const int max) {
        return RandInt(0, max);
    }

    inline float RandFloat(const float max) {
        return RandFloat(0.0f, max);
    }

    inline float RandDir(void) {
        return RandFloat(0.0f, 2.0f * g_pi);
    }

    inline bool Chance(const float perc) {
        assert(perc >= 0.0f && perc <= 1.0f);
        return RandPerc() < perc;
    }
}
