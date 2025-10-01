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

#define NUM_STRIPS 8
#define START_PIN 2
#define STRIP_LEN 137
#define BRIGHTNESS 8
#define FPS 60

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

    ColorMap colorMap({RGB::Red, RGB::Orange, RGB::Yellow, RGB::Green,
                    RGB::Blue, RGB::Indigo, RGB::Violet});
    int nColors = colorMap.getUsed();
    //
    // Print the values that we expec the logic analyzer to be able to pick out.
    for (int i = 0; i < nColors; i++) {
        printf("%06X\n", colorMap[i].getColor(ColorOrder::OGRB));
    }
    colorMap.dim(200);

    //
    // Print the values that we expec the logic analyzer to be able to pick out.
    for (int i = 0; i < nColors; i++) {
        printf("%06X\n", colorMap[i].getColor(ColorOrder::OGRB));
    }
 
    Canvas c2(STRIP_LEN);
    for (int i = 0; i < ns; i++) {
        c2.add(*strips[i]);
    }
    c2.setup();
    c2.setColorMap(&colorMap);
    c2.clear();
    c2.show();


    BarberPole bp2(&c2, nColors, 5);

    Animator a2(&c2, 5);
    a2.add(&bp2);
    a2.init();
    a2.setFPS(FPS);


    while (1) {
        a2.step();
        if (a2.getFrameCount() % 200 == 0) {
            a2.printStats();
        }
    }
}
