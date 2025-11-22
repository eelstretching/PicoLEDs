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

    ArrayColorMap fadeMap(128);
    fadeMap.setBrightness(BRIGHTNESS);

    Canvas c(CANVAS_WIDTH);
    for (int i = 0; i < ns; i++) {
        c.add(*strips[i]);
    }
    
    c.setup();
    c.setColorMap(&fadeMap);


    while (1) {
        for(int i = 0; i < fadeMap.getSize(); i++) {
            c.fill(i);
            c.show();
            sleep_ms(2000);
        }
    }
}
