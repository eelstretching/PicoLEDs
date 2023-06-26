#include <stdlib.h>

#include "Strip.h"
#include "StopWatch.h"
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

    strip.setFractionalBrightness(16);

    int delay = 500;
    int n = 0;

    while (1) {
        strip.fill(RGB::Red);
        strip.show();
        sleep_ms(delay);

        strip.fill(RGB::Green);
        strip.show();
        sleep_ms(delay);

        strip.fill(RGB::Blue);
        strip.show();
        sleep_ms(delay);

        n++;
        if(n % 10 == 0) {
            StopWatch sw = strip.getStripStats();
            printf("%d loops, %.2f us per show\n", n, sw.getAverageTime());
        }

    }

}
