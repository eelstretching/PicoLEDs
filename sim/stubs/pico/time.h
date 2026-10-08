// Desktop simulator stand-in for the Pico SDK's pico/time.h. The clock is
// provided by the simulator: real time when there's a window, and a virtual
// clock that only moves when the program sleeps or renders when we're
// recording a GIF.
#ifndef PICOLEDS_SIM_PICO_TIME_H
#define PICOLEDS_SIM_PICO_TIME_H

#include "pico/types.h"

uint64_t time_us_64(void);
void sleep_us(uint64_t us);

static inline uint32_t time_us_32(void) { return (uint32_t)time_us_64(); }
static inline void sleep_ms(uint32_t ms) { sleep_us((uint64_t)ms * 1000); }
static inline void busy_wait_us(uint64_t us) { sleep_us(us); }
static inline void busy_wait_ms(uint32_t ms) { sleep_ms(ms); }
static inline absolute_time_t get_absolute_time(void) { return time_us_64(); }
static inline uint32_t to_ms_since_boot(absolute_time_t t) { return (uint32_t)(t / 1000); }
static inline uint64_t to_us_since_boot(absolute_time_t t) { return t; }
static inline absolute_time_t make_timeout_time_ms(uint32_t ms) { return time_us_64() + (uint64_t)ms * 1000; }
static inline absolute_time_t make_timeout_time_us(uint64_t us) { return time_us_64() + us; }
static inline int64_t absolute_time_diff_us(absolute_time_t from, absolute_time_t to) { return (int64_t)(to - from); }
static inline void sleep_until(absolute_time_t t) {
    uint64_t now = time_us_64();
    if (t > now) sleep_us(t - now);
}

typedef int32_t alarm_id_t;

#endif
