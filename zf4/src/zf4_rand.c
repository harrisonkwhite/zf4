#include <zf4_rand.h>

#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>

static bool i_rng_initialized;

void InitRNG(void) {
    assert(!i_rng_initialized);
    srand(time(NULL));
    i_rng_initialized = true;
}

int RandInt(const int min, const int max) {
    assert(i_rng_initialized);
    assert(max >= min);

    return min + (rand() % (max - min + 1));
}

float RandFloat(const float min, const float max) {
    assert(i_rng_initialized);
    assert(max >= min);

    const float range = max - min;
    return min + (range * ((float)rand() / RAND_MAX));
}

float RandPerc(void) {
    assert(i_rng_initialized);
    return (float)rand() / RAND_MAX;
}
