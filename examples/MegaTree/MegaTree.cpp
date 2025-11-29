#include <stdlib.h>
#include <string>

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
#include <RandomAnimation.h>

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

    Canvas canvas(CANVAS_WIDTH);
    for (int i = 0; i < ns; i++) {
        canvas.add(*strips[i]);
    }
    canvas.setup();
    canvas.setColorMap(&dimXmasColors);

    for(int i = 0; i < 3; i++) {
        canvas.fill(i+1);
        canvas.show();
        sleep_ms(500);
    }

    canvas.clear();
    canvas.show();

    Animator animator(&canvas, &dimXmasColors, FPS);

    RandomAnimation randimation(&canvas, &midXmasColors);

    ColorCone cone(&canvas, &midXmasColors);
    cone.setName("Cone");
    randimation.addTimed(&cone, 10000);

    Marquees marq(&canvas, &midXmasColors, midXmasColors.getSize(), 20, RIGHT, canvas.getHeight());
    marq.setName("Marquee");
    randimation.addTimed(&marq, 10000); 

    LinesFill lfu(&canvas, &midXmasColors, midXmasColors.getSize(), UP, 1);
    lfu.setName("LFU");
    lfu.setGap(6);
    randimation.addTimed(&lfu, 10000);

    LinesFill lfd(&canvas, &midXmasColors, midXmasColors.getSize(), DOWN, 1);
    lfd.setName("LFD");
    lfd.setGap(7);
    randimation.addTimed(&lfd, 10000);

    Spiral spiral(&canvas, &midXmasColors, 10, 25);
    spiral.setName("Spiral");
    randimation.addTimed(&spiral, 10000);

    ArrayColorMap icicleMap(8);
    Icicles icicles(&canvas, &icicleMap, 10, 6, RGB(128, 128, 128));
    icicles.setName("Icicles");
    randimation.addTimed(&icicles, 20000);

    ColorBars cb1(&canvas, &midXmasColors, 20, 2);
    cb1.setName("CB1");
    randimation.addTimed(&cb1, 10000);

    ColorBars cb2(&canvas, &midXmasColors, 15, 3);
    cb2.setName("CB2");
    randimation.addTimed(&cb2, 10000);

    FadingBars fb1(&canvas, &brightXmasColors, 20, 2);
    fb1.setName("FB1");
    randimation.addTimed(&fb1, 10000);

    ArrayColorMap blucicleMap(8);
    Icicles blucicles(&canvas, &blucicleMap, 10, 6, RGB(0, 0, 128));
    blucicles.setName("Blucicles");
    randimation.addTimed(&blucicles, 20000);

    FadingBars fb2(&canvas, &brightXmasColors, 10, 4);
    fb2.setName("FB2");
    randimation.addTimed(&fb2, 10000);

    FadingBars fb3(&canvas, &brightXmasColors, 4, 2);
    fb3.setName("FB3");
    randimation.addTimed(&fb3, 10000);

    FadingBars fb4(&canvas, &brightXmasColors, 4, 6);
    fb4.setName("FB4");
    randimation.addTimed(&fb4, 10000);

    RotatingColumns rc1(&canvas, &midXmasColors, 20, 2);
    rc1.setName("RC1");
    randimation.addTimed(&rc1, 10000);

    RotatingColumns rc2(&canvas, &midXmasColors, 10, 4);
    rc2.setName("RC2");
    randimation.addTimed(&rc2, 10000);

    RotatingRows rr1(&canvas, &midXmasColors, 4, 2);
    rr1.setName("RR1");
    randimation.addTimed(&rr1, 10000);

    RotatingRows rr2(&canvas, &midXmasColors, 4, 5);
    rr2.setName("RR2");
    randimation.addTimed(&rr2, 10000);

    animator.add(&randimation);

    animator.init();

    while (true) {
        animator.step(); 
        if (animator.getFrameCount() % 200 == 0) {
            animator.printStats();
        }
    }
}
