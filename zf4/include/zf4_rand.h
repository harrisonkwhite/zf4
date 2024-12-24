#ifndef ZF4_RAND_H
#define ZF4_RAND_H

void zf4_init_rng();
int zf4_gen_rand_int(const int min, const int max);
float zf4_gen_rand_float(const float min, const float max);
float zf4_gen_rand_perc();

#endif
