#include <stdlib.h>

#include "Animator.h"
#include "BarberPole.h"
#include "Bouncer.h"
#include "Canvas.h"
#include "Strip.h"
#include "colorutils.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/printf.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/types.h"
#include <ArrayColorMap.h>

#define NUM_STRIPS 16
#define START_PIN 2
#define STRIP_LEN 136
#define FPS 80
#define BRIGHTNESS 32

int main() {
    stdio_init_all();

    //
    // A canvas made out of strips.
    Strip *strips[NUM_STRIPS];
    int ns = NUM_STRIPS;
    int pin = START_PIN;
    int pins[NUM_STRIPS];
    for (int i = 0; i < ns; i++) {
        pins[i] = pin;
        strips[i] = new Strip(pin++, STRIP_LEN);
    }

    ArrayColorMap colorMap({
        RGB::Black, RGB::Red,  RGB::Orange, RGB::Yellow, RGB::Green,
        RGB::Blue,  RGB::Indigo, RGB::Violet
    });
    int nColors = colorMap.getUsed();
    //
    // Print the values that we expec the logic analyzer to be able to pick out.
    for (int i = 0; i < nColors; i++) {
        printf("%06X\n", colorMap[i].getColor(ColorOrder::OGRB));
    }
    printf("\nBrightness 64\n");
    for (int i = 0; i < nColors; i++) {
        printf("%06X\n", colorMap[i].scale8(64).getColor(ColorOrder::OGRB));
    }
    printf("\nBrightness 32\n");
    for (int i = 0; i < nColors; i++) {
        printf("%06X\n", colorMap[i].scale8(32).getColor(ColorOrder::OGRB));
    }
    printf("\nBrightness 16\n");
    for (int i = 0; i < nColors; i++) {
        printf("%06X\n", colorMap[i].scale8(16).getColor(ColorOrder::OGRB));
    }

    printf("\nBrightness 8\n");
    for (int i = 0; i < nColors; i++) {
        printf("%06X\n", colorMap[i].scale8(8).getColor(ColorOrder::OGRB));
    }

    Renderer r1;
    Renderer r2;

    r1.add(*strips[0]);
    r1.add(*strips[1]);
    r1.add(*strips[2]);
    r1.add(*strips[3]);
    r1.add(*strips[4]);
    r1.add(*strips[5]);
    r1.add(*strips[6]);
    r1.add(*strips[7]);
    for (int i = 8; i < ns; i++) {
        r2.add(*strips[i]);
    }
    // r2.add(*strips[8]);
    // r2.add(*strips[9]);
    // r2.add(*strips[10]);
    // r2.add(*strips[12]);
    // r2.add(*strips[13]);
    // r2.add(*strips[14]);

    int starts[ns];
    for (int i = 0; i < ns; i++) {
        starts[i] = i;
    }
    StopWatch frameWatch;
    float usPerFrame = 1e6 / FPS;

    while (1) {
        frameWatch.start();
        int ci = 0;
        for (int i = 0; i < ns; i++) {
            for (int j = 0; j < STRIP_LEN; j+=4) {
                int p = (j + starts[i]) % STRIP_LEN;
                for (int s = 0; s < 4; s++) {
                    strips[i]->putPixel(ci+1, p);
                    p = (p + 1) % STRIP_LEN;
                }
                ci = (ci + 1) % nColors;
            }
            starts[i] = (starts[i] + 1) % STRIP_LEN;
        }
        r1.render(&colorMap);
        r2.render(&colorMap);

        //
        // Wait until we need to render the next frame.
        uint64_t lus = frameWatch.getLastTime();
        if (lus < usPerFrame) {
            sleep_us(usPerFrame - lus);
        }
    }
}
