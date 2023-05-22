/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>

#include "Canvas.h"
#include "Strip.h"
#include "View.h"
#include "colorutil.h"
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

    //
    // Init to clear the strips and show they're working while rendering's
    // busted.
    for (int i = 0; i < 4; i++) {
        strips[i].fill(RGB::Green);
        strips[i].show();
    }
    for (int i = 0; i < 4; i++) {
        sleep_ms(50);
        strips[i].fill(RGB::Black, 0, 138);
        strips[i].show();

        sleep_ms(50);
        strips[i].fill(RGB::Black, 138, 276);
        strips[i].show();
    }

    canvas.clear();

    //
    // Make a gradient that's 16 wide.
    RGB grad1[16];
    fill_gradient_RGB(grad1, 0, RGB::Blue, 16, RGB::Green);
    RGB grad2[16];
    fill_gradient_RGB(grad2, 0, RGB::Yellow, 16, RGB::Red);

    //
    // Make a 4x16 block of the gradients
    int n = 0;

    canvas.clear();
    canvas.copy(grad1, 16, 40, 0);
    canvas.copy(grad2, 16, 40, 1);
    canvas.copy(grad1, 16, 40, 2);
    canvas.copy(grad2, 16, 40, 3);

    canvas.show();
    sleep_ms(200);
    while (1) {
        int rx = 40;
        int ry = 0;
        for (int i = 0; i < 40; i++) {
            canvas.shiftRight(rx++, 0, 16, 4, 1);
            canvas.show();
            sleep_ms(50);
        }

        for (int i = 0; i < 40; i++) {
            canvas.shiftLeft(rx--, 0, 16, 4, 1);
            canvas.show();
            sleep_ms(50);
        }

        if(ry + 5 < canvas.getHeight()) {
            canvas.shiftUp(rx, ry, 16, 4, 1);
            canvas.show();
            sleep_ms(50);
            ry++;
        } else {
            canvas.shiftDown(rx, ry, 16, 4, 1);
            canvas.show();
            sleep_ms(50);
            ry--;
        }

        n++;
        if (n % 20 == 0) {
            StopWatch *stats = canvas.getStopWatch();
            printf("Average show time is %.2f us\n", stats->getAverageTime());
        }
    }
}
