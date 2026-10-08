// Desktop simulator stand-in for the Pico SDK's hardware/clocks.h. We report
// the RP2350's default system clock.
#ifndef PICOLEDS_SIM_HARDWARE_CLOCKS_H
#define PICOLEDS_SIM_HARDWARE_CLOCKS_H

#include "pico/types.h"

enum clock_index { clk_gpout0 = 0, clk_ref, clk_sys, clk_peri, clk_usb, clk_adc };

static inline uint32_t clock_get_hz(enum clock_index clk) {
    (void)clk;
    return 150000000;
}

#endif
