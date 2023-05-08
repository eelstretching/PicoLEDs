/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>

#include "Strip.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/printf.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/types.h"
#include "ws2812.pio.h"

void pattern_snakes(Strip strip, uint t) {
    strip.reset();
    for (uint i = 0; i < strip.getNumPixels(); ++i) {
        uint x = (i + (t >> 1)) % 64;
        if (x < 10) {
            strip.addPixel(RGB::Red);
        } else if (x >= 15 && x < 25) {
            strip.addPixel(RGB::Green);
        } else if (x >= 30 && x < 40) {
            strip.addPixel(RGB::Blue);
        } else {
            strip.addPixel(RGB::Black);
        }
    }
    strip.show();
}

int main() {
    stdio_init_all();

    Strip strips[4] = {Strip(2, 276), Strip(3, 276), Strip(4, 276),
                       Strip(5, 276)};

    RGB colors[6] = {RGB::Red,    RGB::Green,    RGB::Blue,
                     RGB::Purple, RGB::DeepPink, RGB::White};

    int c = 0;
    int n = 0;
    while (1) {
        for (int i = 0; i < 4; i++) {
            strips[i].fill(colors[c]);
            strips[i].show();
            c = (c + 1) % 6;
            n++;
        }
        sleep_ms(500);
        if(n % 500 == 0) {
            StripStats cs;
            for(int i = 0; i < 4; i++) {
                cs.combine(strips[i].getStripStats());
            }
            printf("%d shows, %.2f us per show\n", cs.showCount, ((double) cs.showTime)/cs.showCount);
        }
    }
}
