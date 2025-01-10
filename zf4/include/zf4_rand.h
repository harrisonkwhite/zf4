#pragma once

namespace zf4 {
    void init_rng();
    int gen_rand_int(const int min, const int max);
    float gen_rand_float(const float min, const float max);
    float gen_rand_perc();

    inline int gen_rand_int(const int max) {
        return gen_rand_int(0, max);
    }

    inline float gen_rand_float(const float max) {
        return gen_rand_float(0.0f, max);
    }
}
