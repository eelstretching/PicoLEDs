// Desktop simulator stand-in for the Pico SDK's pico/types.h.
#ifndef PICOLEDS_SIM_PICO_TYPES_H
#define PICOLEDS_SIM_PICO_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

typedef unsigned int uint;
typedef unsigned short ushort;

typedef uint64_t absolute_time_t;

#ifndef MIN
#define MIN(a, b) ((b) > (a) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#endif
