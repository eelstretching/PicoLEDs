#include <stdlib.h>
#include <string>

#include "Animator.h"
#include "ArrayColorMap.h"
#include "BarberPole.h"
#include "Bouncer.h"
#include "Canvas.h"
#include "Strip.h"
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
        RGB::Purple,
        RGB::Orange,
        RGB::Silver,
        RGB::FairyLightNCC,
        RGB(255, 20, 20),    // Bright Red
        RGB(28, 191, 38),    // Vibrant Green
        RGB(0, 33, 111),     // Navy
        RGB(255, 252, 245),  // Warm White
        RGB(213, 181, 52),   // Gold
        RGB(242, 18, 18),    // Deep Red
    });

    ArrayColorMap dimXmasColors(xmasColors);
    dimXmasColors.setBrightness(32);

    ArrayColorMap midXmasColors(xmasColors);
    midXmasColors.setBrightness(64);

    ArrayColorMap brightXmasColors(xmasColors);
    brightXmasColors.setBrightness(128);

    uint8_t rgColors[] = {0, 1};
    uint8_t rwColors[] = {0, 3};
    uint8_t rgwColors[] = {0, 1, 3};
    uint8_t rgbwColors[] = {0, 1, 2, 3};
    uint8_t rgbwgColors[] = {0, 1, 2, 3, 13};


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

    Marquees fancyMarq(&canvas, &midXmasColors, 5, rgbwgColors, 20, RIGHT,
                   canvas.getHeight());
    fancyMarq.setName("Fancy Marquee");
    randimation.addTimed(&fancyMarq, 30000);

    Marquees rwMarq(&canvas, &midXmasColors, 2, rwColors, 20, RIGHT,
                  canvas.getHeight());
    rwMarq.setName("RW Marquee");
    randimation.addTimed(&rwMarq, 30000); 
    
    Marquees rgMarq(&canvas, &midXmasColors, 2, rgColors, 20, RIGHT,
                  canvas.getHeight());
    rgMarq.setName("RG Marquee");
    randimation.addTimed(&rgMarq, 30000); 
    
    LinesFill lfu(&canvas, &midXmasColors, 5, rgbwgColors, UP, 1);
    lfu.setName("LFU");
    lfu.setGap(6);
    randimation.addTimed(&lfu, 10000);

    LinesFill lfd(&canvas, &midXmasColors, 5, rgbwgColors, DOWN, 1);
    lfd.setName("LFD");
    lfd.setGap(7);
    randimation.addTimed(&lfd, 10000);

    Spiral sp1(&canvas, &brightXmasColors, 4, rgbwColors, 10, 25);
    sp1.setName("Spiral 1");
    randimation.addTimed(&sp1, 10000);

    Spiral sp2(&canvas, &brightXmasColors, 1, rwColors, 10, 25);
    sp2.setName("Spiral 2");
    randimation.addTimed(&sp2, 10000);

    Spiral sp3(&canvas, &brightXmasColors, 2, rwColors, 10, 25);
    sp3.setName("Spiral 3");
    randimation.addTimed(&sp3, 10000);

    ArrayColorMap icicleMap(8);
    Icicles icicles(&canvas, &icicleMap, 10, 6, RGB(128, 128, 128));
    icicles.setName("Icicles");
    randimation.addTimed(&icicles, 40000);

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
    randimation.addTimed(&blucicles, 40000);

    FadingBars fb2(&canvas, &brightXmasColors, 10, 4);
    fb2.setName("FB2");
    randimation.addTimed(&fb2, 10000);

    FadingBars fb3(&canvas, &brightXmasColors, 4, 2);
    fb3.setName("FB3");
    randimation.addTimed(&fb3, 10000);

    FadingBars fb4(&canvas, &brightXmasColors, 4, 6);
    fb4.setName("FB4");
    randimation.addTimed(&fb4, 10000);

    RotatingColumns rc1(&canvas, &midXmasColors, 2, rgColors, 20);
    rc1.setName("RC1");
    randimation.addTimed(&rc1, 10000);

    RotatingColumns rc2(&canvas, &midXmasColors, 4, rgbwColors, 10);
    rc2.setName("RC2");
    randimation.addTimed(&rc2, 10000);

    RotatingRows rr1(&canvas, &midXmasColors, 2, rgColors, 4);
    rr1.setName("RR1");
    randimation.addTimed(&rr1, 10000);

    RotatingRows rr2(&canvas, &midXmasColors, 4, rgbwColors, 5);
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
