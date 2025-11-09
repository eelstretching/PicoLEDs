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

#define NUM_STRIPS 7
#define START_PIN 2
#define STRIP_LEN 400
#define CANVAS_WIDTH 100
#define DELAY_MS 100

#define BRIGHTNESS 16
#define FPS 30

int main() {
    stdio_init_all();

    //
    // A canvas made out of strips.
    Strip* strips[NUM_STRIPS];
    int ns = NUM_STRIPS;
    int pin = START_PIN;
    int pins[NUM_STRIPS];
    for (int i = 0; i < ns; i++) {
        pins[i] = pin;
        strips[i] = new Strip(pin++, STRIP_LEN, WS2811);
    }

    ColorMap simpleMap({RGB::Red, RGB::Green, RGB::Blue, RGB::White,
                        RGB::DarkViolet, RGB::Orange, RGB::Yellow, RGB::Cyan});
    simpleMap.setBrightness(BRIGHTNESS);
    int nColors = simpleMap.getUsed();

    Canvas c(CANVAS_WIDTH);
    for (int i = 0; i < ns; i++) {
        c.add(*strips[i]);
    }
    c.setup();
    c.setColorMap(&simpleMap);
    c.clear();
    c.show();

    int cn = 0;
    while (1) {
        for (int i = 0; i < c.getHeight(); i++) {
            for (int j = 0; j < c.getHeight(); j++) {
                c.clear();
                for (int k = 0; k < i; k++) {
                    c.fillRow((k + j) % c.getHeight(), cn);
                }
                c.show();

                sleep_ms(DELAY_MS);
            }
            cn = (cn + 1) % nColors;
        }
    }
}
