// Desktop simulator stand-in for the Pico SDK's pico/stdio.h.
#ifndef PICOLEDS_SIM_PICO_STDIO_H
#define PICOLEDS_SIM_PICO_STDIO_H

#include <stdio.h>

static inline bool stdio_init_all(void) { return true; }
static inline void stdio_flush(void) { fflush(stdout); }

#endif
