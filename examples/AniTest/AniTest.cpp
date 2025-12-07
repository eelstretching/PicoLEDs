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

#define NUM_STRIPS 4
#define START_PIN 2
#define STRIP_LEN 200
#define CANVAS_WIDTH 100

#define BRIGHTNESS 16
#define FPS 30

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
        strips[i] = new Strip(pin++, STRIP_LEN, WS2811);
    }

    ArrayColorMap colorMap({RGB::Red, RGB::Orange, RGB::Yellow, RGB::Green,
                    RGB::Blue, RGB::Indigo, RGB::Violet});
    int nColors = colorMap.getUsed();
    colorMap.setBrightness(BRIGHTNESS);

    ArrayColorMap simpleMap({RGB::Red, RGB::Green, RGB::Blue});
    simpleMap.setBrightness(BRIGHTNESS);
    // nColors = simpleMap.getUsed();

    Canvas c(CANVAS_WIDTH);
    for (int i = 0; i < ns; i++) {
        c.add(*strips[i]);
    }
    c.setup();
    c.setColorMap(&colorMap);
    c.clear();
    c.show();


    BarberPole bp(&c, &colorMap, nColors, 5);

    Animator a(&c, 5);
    a.add(&bp);
    a.init();
    a.setFPS(FPS);


    while (1) {
        a.step();
        if (a.getFrameCount() % 500 == 0) {
            a.printStats();
        }
    }
}
