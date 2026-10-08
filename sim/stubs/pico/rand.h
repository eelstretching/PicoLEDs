// Desktop simulator stand-in for the Pico SDK's pico/rand.h. Seeded from the
// command line so recordings are repeatable.
#ifndef PICOLEDS_SIM_PICO_RAND_H
#define PICOLEDS_SIM_PICO_RAND_H

#include "pico/types.h"

uint32_t get_rand_32(void);

static inline uint64_t get_rand_64(void) {
    return ((uint64_t)get_rand_32() << 32) | get_rand_32();
}

#endif
