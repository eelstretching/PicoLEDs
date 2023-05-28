/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>

#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/printf.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/types.h"

#include "Canvas.h"
#include "Font.h"
#include "FontRobotron.h"
#include "Strip.h"
#include "View.h"
#include "colorutil.h"

int main() {
    stdio_init_all();

    //
    // A canvas and a view made out of strips. Note that the canvas is larger
    // than the view! The view only covers the lower half of the canvas!
    Canvas canvas(138, 16);
    View view(138);
    Strip strips[] = {Strip(2, 276), Strip(3, 276), Strip(4, 276),
                      Strip(5, 276)};
    view.add(strips[0]);
    view.add(strips[1]);
    view.add(strips[2]);
    view.add(strips[3]);
    canvas.setView(&view, 0, 0);
    int delay = 30;

    //
    // Init to clear the strips and show they're working while rendering's
    // busted.
    for (int i = 0; i < 4; i++) {
        strips[i].fill(RGB::Green);
        strips[i].show();
    }
    for (int i = 0; i < 4; i++) {
        sleep_ms(delay);
        strips[i].fill(RGB::Black, 0, 138);
        strips[i].show();

        sleep_ms(delay);
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

    RGB simple[] = {RGB::Red,  RGB::Orange, RGB::Yellow, RGB::Green,
                    RGB::Blue, RGB::Indigo, RGB::Violet, RGB::White};
    char b[30];

    Font robo(&canvas, RobotronFontData);

    uint tw = robo.render("TROOP 103", 5, 0, RGB::Green);
    canvas.show();
    sleep_ms(10000);
    robo.render("BTON", tw+robo.getSpacing(), 0, RGB::Red);
    canvas.show();
    sleep_ms(10000);

 

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
    int rx = 40;
    int ry = 0;
    bool dir = true;
    while (1) {
        printf("Bounce %d %d\n", rx, ry);
        for (int i = 0; i < 40; i++) {
            canvas.shiftRight(rx++, ry, 16, 4, 1);
            canvas.show();
            sleep_ms(delay);
        }

        if (dir) {
            canvas.shiftUp(rx, ry, 16, 4, 1);
            canvas.show();
            sleep_ms(delay);
            ry++;
            if (ry == canvas.getHeight() - 4) {
                dir = false;
            }
        } else {
            canvas.shiftDown(rx, ry, 16, 4, 1);
            canvas.show();
            sleep_ms(delay);
            ry--;
            if (ry == 0) {
                dir = true;
            }
        }

        for (int i = 0; i < 40; i++) {
            canvas.shiftLeft(rx--, ry, 16, 4, 1);
            canvas.show();
            sleep_ms(delay);
        }

        if (dir) {
            canvas.shiftUp(rx, ry, 16, 4, 1);
            canvas.show();
            sleep_ms(delay);
            ry++;
            if (ry == canvas.getHeight() - 4) {
                dir = false;
            }
        } else {
            canvas.shiftDown(rx, ry, 16, 4, 1);
            canvas.show();
            sleep_ms(delay);
            ry--;
            if (ry == 0) {
                dir = true;
            }
        }

        n++;
        if (n % 20 == 0) {
            StopWatch *stats = canvas.getStopWatch();
            printf("Average show time is %.2f us\n", stats->getAverageTime());
        }
    }
}
