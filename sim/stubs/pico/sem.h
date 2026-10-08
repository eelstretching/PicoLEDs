// Desktop simulator stand-in for the Pico SDK's pico/sem.h. The simulated
// Renderer never blocks, so these don't need to do anything.
#ifndef PICOLEDS_SIM_PICO_SEM_H
#define PICOLEDS_SIM_PICO_SEM_H

#include "pico/types.h"

typedef struct {
    int16_t permits;
    int16_t max_permits;
} semaphore_t;
typedef semaphore_t semaphore;

static inline void sem_init(semaphore_t* s, int16_t initial, int16_t max) {
    s->permits = initial;
    s->max_permits = max;
}
static inline void sem_acquire_blocking(semaphore_t* s) { (void)s; }
static inline bool sem_release(semaphore_t* s) { (void)s; return true; }
static inline bool sem_try_acquire(semaphore_t* s) { (void)s; return true; }

#endif
