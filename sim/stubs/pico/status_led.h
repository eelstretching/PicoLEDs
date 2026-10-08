// Desktop simulator stand-in for the Pico SDK's pico/status_led.h.
#ifndef PICOLEDS_SIM_PICO_STATUS_LED_H
#define PICOLEDS_SIM_PICO_STATUS_LED_H

#include "pico/types.h"

static inline bool status_led_init(void) { return true; }
static inline void status_led_set_state(bool on) { (void)on; }
static inline bool status_led_get_state(void) { return false; }
static inline void status_led_deinit(void) {}

#endif
