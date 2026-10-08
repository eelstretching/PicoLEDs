// Desktop simulator stand-in for the Pico SDK's pico/platform.h.
#ifndef PICOLEDS_SIM_PICO_PLATFORM_H
#define PICOLEDS_SIM_PICO_PLATFORM_H

#include <stdio.h>
#include <stdlib.h>

#include "pico/types.h"

#define __not_in_flash_func(f) f
#define __time_critical_func(f) f
#define __no_inline_not_in_flash_func(f) f
#define __not_in_flash(group)
#define __in_flash(group)
#define __scratch_x(group)
#define __scratch_y(group)

#define hard_assert(x) \
    do {               \
        if (!(x)) {    \
            fprintf(stderr, "hard_assert failed: %s (%s:%d)\n", #x, __FILE__, __LINE__); \
            abort();   \
        }              \
    } while (0)

#define panic(...)                    \
    do {                              \
        fprintf(stderr, "panic: ");   \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n");        \
        abort();                      \
    } while (0)

static inline void tight_loop_contents(void) {}
static inline uint get_core_num(void) { return 0; }

#endif
