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
        HSV(0, 0, 0)         // Black
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
    TimedAnimation ta1 = TimedAnimation(&cone, 10000);
    a.add(&ta1);

    Marquees marq(&c, 4, 20, RIGHT, c.getHeight());
    TimedAnimation ta2 = TimedAnimation(&marq, 10000);
    a.add(&ta2); 

    LinesFill lfu(&c, 4, UP, 1);
    lfu.setGap(6);
    TimedAnimation ta3 = TimedAnimation(&lfu, 10000);
    a.add(&ta3);

    LinesFill lfd(&c, 4, DOWN, 1);
    lfd.setGap(7);
    TimedAnimation ta4 = TimedAnimation(&lfd, 10000);
    a.add(&ta4);

    Spiral spiral(&c, 5, 25);
    TimedAnimation ta5 = TimedAnimation(&spiral, 10000);
    a.add(&ta5);

    // TripleColor tc(&c, simpleXmasColors);
    // a.add(&tc);
    a.init();

    while (true) {
        a.step();
        if (a.getFrameCount() % 200 == 0) {
            a.printStats();
        }
    }
}
