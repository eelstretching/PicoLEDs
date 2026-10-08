// Desktop simulator stand-in for the Pico SDK's hardware/gpio.h. There are no
// pins on a Mac, so these do nothing.
#ifndef PICOLEDS_SIM_HARDWARE_GPIO_H
#define PICOLEDS_SIM_HARDWARE_GPIO_H

#include "pico/types.h"

#define GPIO_IN 0
#define GPIO_OUT 1

static inline void gpio_init(uint pin) { (void)pin; }
static inline void gpio_set_dir(uint pin, bool out) { (void)pin; (void)out; }
static inline void gpio_put(uint pin, bool value) { (void)pin; (void)value; }
static inline bool gpio_get(uint pin) { (void)pin; return false; }
static inline void gpio_pull_up(uint pin) { (void)pin; }
static inline void gpio_pull_down(uint pin) { (void)pin; }

#endif
