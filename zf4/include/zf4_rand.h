#pragma once

namespace zf4 {
    void InitRNG(void);
    int RandInt(const int min, const int max);
    float RandFloat(const float min, const float max);
    float RandPerc(void);
}
