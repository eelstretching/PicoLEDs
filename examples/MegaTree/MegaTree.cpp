#include <stdlib.h>

#include "Animator.h"
#include "BarberPole.h"
#include "Bouncer.h"
#include "Canvas.h"
#include "Strip.h"
#include "TripleColor.h"
#include "Spiral.h"
#include "LinesFill.h"
#include "Marquees.h"
#include "ColorCone.h"
#include "colorutils.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/printf.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/types.h"
#include <TimedAnimation.h>

#define NUM_STRIPS 7
#define START_PIN 2
#define STRIP_LEN 400
#define CANVAS_WIDTH 100
#define BRIGHTNESS 32
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
        strips[i] = new Strip(pin++, STRIP_LEN);
    }

    //
    // We want to operate in HSV space so that dimming is easy.
    ColorMap xmasColors({
        HSV(0,0,0),
        HSV(139, 179, 242),  // Pine
        HSV(0, 100, 100),    // Red
        HSV(85, 100, 100),   // Green
        HSV(170, 100, 100),  // Blue
        HSV(42, 100, 100),   // Yellow
        HSV(128, 100, 100),  // Cyan
        HSV(213, 100, 100),  // Magenta
    });

    ColorMap simpleXmasColors({
        RGB::Red,
        RGB::Green,
        RGB::Blue,
        RGB::White, 
        RGB::Yellow,
        RGB::Orange
    });

    simpleXmasColors.setBrightness(BRIGHTNESS);

    Canvas c(CANVAS_WIDTH);
    for (int i = 0; i < ns; i++) {
        c.add(*strips[i]);
    }
    c.setup();
    c.setColorMap(&simpleXmasColors);

    for(int i = 0; i < 3; i++) {
        c.fill(i+1);
        c.show();
        sleep_ms(500);
    }

    c.clear();
    c.show();

    Animator a(&c, FPS);

    ColorCone cone(&c, &simpleXmasColors);
    a.addTimed(&cone, 10000);

    Marquees marq(&c, simpleXmasColors.getSize(), 20, RIGHT, c.getHeight());
    a.addTimed(&marq, 10000); 

    LinesFill lfu(&c, simpleXmasColors.getSize(), UP, 1);
    lfu.setGap(6);
    a.addTimed(&lfu, 10000);

    LinesFill lfd(&c, simpleXmasColors.getSize(), DOWN, 1);
    lfd.setGap(7);
    a.addTimed(&lfd, 10000);

    Spiral spiral(&c, &simpleXmasColors, 10, 25);
    a.addTimed(&spiral, 10000);

    a.init();

    while (true) {
        a.step();
        if (a.getFrameCount() % 200 == 0) {
            a.printStats();
        }
    }
}
