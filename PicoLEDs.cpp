/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "Strip.h"
#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "pico/printf.h"
#include "pico/types.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

void pattern_snakes(Strip strip, uint t) {
    for (uint i = 0; i < strip.getNumPixels(); ++i) {
        uint x = (i + (t >> 1)) % 64;
        if (x < 10) {
            uint p = strip.addPixel((uint8_t) 0xff, (uint8_t) 0, (uint8_t) 0);
        } else if (x >= 15 && x < 25) {
            uint p = strip.addPixel((uint8_t) 0, (uint8_t) 0xff, (uint8_t) 0);
        } else if (x >= 30 && x < 40) {
            uint p = strip.addPixel((uint8_t) 0, (uint8_t) 0, (uint8_t) 0xff);
        } else {
            uint p = strip.addPixel((uint8_t) 0, (uint8_t) 0, (uint8_t) 0);
        }
    }
    strip.show();
}

int main() {
    //set_sys_clock_48();
    stdio_init_all();
    printf("Strip Smoke Test, using pin %d", 1);

    Strip strip(1, 32);

    int t = 0;
    while (1) {
        for (int i = 0; i < 1000; ++i) {
            pattern_snakes(strip, t);
            sleep_ms(10);
            t++;
        }
    }
}
