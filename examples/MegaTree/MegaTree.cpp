#include <stdlib.h>

#include "Animator.h"
#include "ArrayColorMap.h"
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
#include "Icicles.h"
#include <ColorBars.h>
#include <FadingBars.h>
#include <RotatingBars.h>

#define NUM_STRIPS 1
#define START_PIN 2
#define STRIP_LEN 300
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

    ArrayColorMap xmasColors({
        RGB::Red,
        RGB::Green,
        RGB::Blue,
        RGB::White, 
        RGB::Yellow,
        RGB::Orange
    });

    ArrayColorMap dimXmasColors(xmasColors);
    dimXmasColors.setBrightness(32);

    ArrayColorMap brightXmasColors(xmasColors);
    brightXmasColors.setBrightness(128);

    Canvas c(CANVAS_WIDTH);
    for (int i = 0; i < ns; i++) {
        c.add(*strips[i]);
    }
    c.setup();
    c.setColorMap(&xmasColors);

    for(int i = 0; i < 3; i++) {
        c.fill(i+1);
        c.show();
        sleep_ms(500);
    }

    c.clear();
    c.show();

    Animator a(&c, FPS);

    // ColorCone cone(&c, &simpleXmasColors);
    // a.addTimed(&cone, 10000);

    // Marquees marq(&c, simpleXmasColors.getSize(), 20, RIGHT, c.getHeight());
    // a.addTimed(&marq, 10000); 

    // LinesFill lfu(&c, simpleXmasColors.getSize(), UP, 1);
    // lfu.setGap(6);
    // a.addTimed(&lfu, 10000);

    // LinesFill lfd(&c, simpleXmasColors.getSize(), DOWN, 1);
    // lfd.setGap(7);
    // a.addTimed(&lfd, 10000);

    // Spiral spiral(&c, &simpleXmasColors, 10, 25);
    // a.addTimed(&spiral, 10000);

    // Icicles icicles(&c, 10, 5, RGB::White);
    // a.add(&icicles);

    // ColorBars cb1(&c, 20, 2);
    // a.addTimed(&cb1, 5000);

    // ColorBars cb2(&c, 15, 3);
    // a.addTimed(&cb2, 5000);

    // FadingBars fb1(&c, 20, 2, &brightXmasColors);
    // a.addTimed(&fb1, 20000);

    // FadingBars fb2(&c, 10, 4, &brightXmasColors);
    // a.addTimed(&fb2, 20000);

    RotatingBars rb1(&c, &brightXmasColors, 20, 2);
    a.addTimed(&rb1, 20000);

    RotatingBars rb2(&c, &brightXmasColors, 10, 4);
    a.addTimed(&rb2, 20000);

    a.init();

    while (true) {
        a.step(); 
        if (a.getFrameCount() % 200 == 0) {
            a.printStats();
        }
    }
}
