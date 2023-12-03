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
    Strip strips[] = {Strip(2, 138), Strip(3, 138)};

    RGB colors[] = {RGB::Red, RGB::Green};

    strips[0].setFractionalBrightness(32);
    strips[1].setFractionalBrightness(32);

    int delay = 5;

    int dirs[] = {1,0};

    int width = 10;

    int posns[] = {0, (int) strips[1].getNumPixels() - width};

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 2; j++) {
            strips[j].fill(RGB::Red);
            strips[j].show();
        }
        sleep_ms(200);
        for (int j = 0; j < 2; j++) {
            strips[j].fill(RGB::Green);
            strips[j].show();
        }
        sleep_ms(200);
        for (int j = 0; j < 2; j++) {
            strips[j].fill(RGB::Blue);
            strips[j].show();
        }
        sleep_ms(200);
        for (int j = 0; j < 2; j++) {
            strips[j].fill(RGB::Black);
            strips[j].show();
        }
        sleep_ms(200);
    }

    StopWatch fw;
    while (1) {
        fw.start();
        for (int s = 0; s < 2; s++) {
            if (dirs[s] == 1) {
                if (posns[s] > 0) {
                    strips[s].putPixel(RGB::Black, posns[s] - 1);
                }
                for (int i = posns[s]; i < posns[s] + width; i++) {
                    strips[s].putPixel(colors[s], i);
                }
                posns[s]++;
                if (posns[s] == strips[s].getNumPixels() - width) {
                    dirs[s] = 0;
                }
            } else {
                if (posns[s] < strips[s].getNumPixels() - width) {
                    strips[s].putPixel(RGB::Black, posns[s] + width);
                }
                for (int i = posns[s]; i < posns[s] + width; i++) {
                    strips[s].putPixel(colors[s], i);
                }
                posns[s]--;
                if (posns[s] == -1) {
                    dirs[s] = 1;
                    posns[s] = 0;
                }
            }
            strips[s].show();
        }
        if (delay > 0) {
            sleep_ms(delay);
        }

        fw.finish();
        if (fw.count % 500 == 0) {
            printf("%d frames, %.2f f/s\n", fw.count,
                   fw.count / (fw.totalTime / 1e6));
        }
    }
}
