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
#include <RotatingRows.h>
#include <RotatingColumns.h>

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
        //
        // First strip is a little short at the moment.
        strips[i] = new Strip(pin++, i == 0 ? STRIP_LEN-100 : STRIP_LEN);
        strips[i]->setColorOrder(ColorOrder::ORGB);
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

    ArrayColorMap midXmasColors(xmasColors);
    midXmasColors.setBrightness(64);

    ArrayColorMap brightXmasColors(xmasColors);
    brightXmasColors.setBrightness(128);

    Canvas c(CANVAS_WIDTH);
    for (int i = 0; i < ns; i++) {
        c.add(*strips[i]);
    }
    c.setup();
    c.setColorMap(&dimXmasColors);

    for(int i = 0; i < 3; i++) {
        c.fill(i+1);
        c.show();
        sleep_ms(500);
    }

    c.clear();
    c.show();

    Animator a(&c, &dimXmasColors, FPS);

    ColorCone cone(&c, &midXmasColors);
    a.addTimed(&cone, 10000);

    Marquees marq(&c, &midXmasColors, midXmasColors.getSize(), 20, RIGHT, c.getHeight());
    a.addTimed(&marq, 10000); 

    LinesFill lfu(&c, &midXmasColors, midXmasColors.getSize(), UP, 1);
    lfu.setGap(6);
    a.addTimed(&lfu, 10000);

    LinesFill lfd(&c, &midXmasColors, midXmasColors.getSize(), DOWN, 1);
    lfd.setGap(7);
    a.addTimed(&lfd, 10000);

    Spiral spiral(&c, &midXmasColors, 10, 25);
    a.addTimed(&spiral, 10000);

    ArrayColorMap icicleMap(8);
    Icicles icicles(&c, &icicleMap, 2, 6, RGB(128, 128, 128));
    a.addTimed(&icicles, 20000);

    ColorBars cb1(&c, &midXmasColors, 20, 2);
    a.addTimed(&cb1, 10000);

    ColorBars cb2(&c, &midXmasColors, 15, 3);
    a.addTimed(&cb2, 10000);

    FadingBars fb1(&c, &brightXmasColors, 20, 2);
    a.addTimed(&fb1, 10000);

    ArrayColorMap blucicleMap(8);
    Icicles blucicles(&c, &blucicleMap, 2, 6, RGB(0, 0, 128));
    a.addTimed(&blucicles, 20000);

    FadingBars fb2(&c, &brightXmasColors, 10, 4);
    a.addTimed(&fb2, 10000);

    FadingBars fb3(&c, &brightXmasColors, 4, 2);
    a.addTimed(&fb3, 10000);

    FadingBars fb4(&c, &brightXmasColors, 4, 6);
    a.addTimed(&fb4, 10000);

    RotatingColumns rc1(&c, &midXmasColors, 20, 2);
    a.addTimed(&rc1, 10000);

    RotatingColumns rc2(&c, &midXmasColors, 10, 4);
    a.addTimed(&rc2, 10000);

    RotatingRows rr1(&c, &midXmasColors, 1, 3);
    a.addTimed(&rr1, 10000);

    a.init();

    while (true) {
        a.step(); 
        if (a.getFrameCount() % 200 == 0) {
            a.printStats();
        }
    }
}
