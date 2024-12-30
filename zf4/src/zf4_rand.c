#include <zf4_rand.h>

#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <stdbool.h>

static bool i_rngInitialized;

void zf4_init_rng() {
    assert(!i_rngInitialized);
    srand((unsigned int)time(NULL));
    i_rngInitialized = true;
}

int zf4_gen_rand_int(const int min, const int max) {
    assert(i_rngInitialized);
    assert(max >= min);

    return min + (rand() % (max - min + 1));
}

float zf4_gen_rand_float(const float min, const float max) {
    assert(i_rngInitialized);
    assert(max >= min);

    const float range = max - min;
    return min + (range * ((float)rand() / RAND_MAX));
}

float zf4_gen_rand_perc() {
    assert(i_rngInitialized);
    return (float)rand() / RAND_MAX;
}
