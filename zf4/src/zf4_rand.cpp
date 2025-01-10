#include <zf4_rand.h>

#include <cstdlib>
#include <ctime>
#include <cassert>

namespace zf4 {
    static bool i_rngInitialized;

    void init_rng() {
        assert(!i_rngInitialized);
        srand(static_cast<unsigned int>(time(nullptr)));
        i_rngInitialized = true;
    }

    int gen_rand_int(const int min, const int max) {
        assert(i_rngInitialized);
        assert(max >= min);

        return min + (rand() % (max - min + 1));
    }

    float gen_rand_float(const float min, const float max) {
        assert(i_rngInitialized);
        assert(max >= min);

        const float range = max - min;
        return min + (range * (static_cast<float>(rand()) / RAND_MAX));
    }

    float gen_rand_perc() {
        assert(i_rngInitialized);
        return static_cast<float>(rand()) / RAND_MAX;
    }
}
