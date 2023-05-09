/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>

#include "Canvas.h"
#include "Strip.h"
#include "View.h"
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

    //
    // A canvas and a view made out of strips.
    Canvas canvas(138, 8);
    View view(138);
    Strip strips[] = {Strip(2, 276), Strip(3, 276), Strip(4, 276),
                      Strip(5, 276)};
    view.add(strips[0]);
    view.add(strips[1]);
    view.add(strips[2]);
    view.add(strips[3]);
    canvas.setView(&view, 0, 0);

    RGB colors[8] = {RGB::Red,  RGB::Orange, RGB::Yellow, RGB::Green,
                     RGB::Blue, RGB::Indigo, RGB::Violet, RGB::White};

    //
    // Init to clear the strips and show they're working while rendering's busted.
    for (int i = 0; i < 4; i++) {
        strips[i].fill(RGB::Coral);
        strips[i].show();
    }
    for (int i = 0; i < 4; i++) {
        sleep_ms(500);
        strips[i].fill(RGB::Black, 0, 138);

        strips[i].show();
        sleep_ms(500);
        strips[i].fill(RGB::Black, 138, 276);
        strips[i].show();
    }
    uint n = 0;
    while (1) {
        if (n % 2 == 0) {
            for (int i = 0; i < 8; i++) {
                canvas.fill(i, colors[i]);
            }
        } else {
            for (int i = 0, c = 7; i < 8; i++, c--) {
                canvas.fill(i, colors[c]);
            }
        }
        n++;
        canvas.show();
        sleep_ms(5000);
    }
}
