#include <stdlib.h>

#include "StopWatch.h"
#include "Strip.h"
#include "colorutils.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/printf.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/types.h"

int main() {
    stdio_init_all();

    //
    // Simple test for a single strip of pixels.
    Strip strip(2, 552);

    strip.setFractionalBrightness(32);

    int delay = 20;

    int n = 0;

    int pos = 0;

    int dir = 1;

    int width = 20;

    for (int i = 0; i < 10; i++) {
        strip.fill(RGB::Red);
        strip.show();
        sleep_ms(200);
        strip.fill(RGB::Green);
        strip.show();
        sleep_ms(200);
        strip.fill(RGB::Blue);
        strip.show();
        sleep_ms(200);
        strip.fill(RGB::Black);
        strip.show();
        sleep_ms(200);
    }

    while (1) {
        if (dir == 1) {
            if (pos > 0) {
                strip.putPixel(RGB::Black, pos - 1);
            }

            for (int i = pos; i < pos + width; i++) {
                strip.putPixel(RGB::Red, i);
            }
            pos++;
            if (pos == strip.getNumPixels() - width) {
                dir = 0;
            }
        } else {
            if (pos < strip.getNumPixels() - width) {
                strip.putPixel(RGB::Black, pos + width);
            }
            for (int i = pos; i < pos + width; i++) {
                strip.putPixel(RGB::Red, i);
            }
            pos--;
            if (pos == width) {
                dir = 1;
            }
        }
        strip.show();
        sleep_ms(delay);

        n++;
        if (n % 10 == 0) {
            StopWatch sw = strip.getStripStats();
            printf("%d loops, %.2f us per show\n", n, sw.getAverageTime());
        }
    }
}
