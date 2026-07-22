#include <Bursts.h>
#include <ColorBars.h>
#include <FadingBars.h>
#include <FontTwoP.h>
#include <PacChase.h>
#include <RandomAnimator.h>
#include <RotRandColumns.h>
#include <RotRandRows.h>
#include <RotatingColumns.h>
#include <RotatingRows.h>
#include <ScrollTexts.h>
#include <TextAnimation.h>
#include <TimedAnimation.h>
#include <XmasTree.h>
#include <stdlib.h>

#include <string>

#include "Animator.h"
#include "ArrayColorMap.h"
#include "BarberPole.h"
#include "Bouncer.h"
#include "Canvas.h"
#include "ColorCone.h"
#include "Ghost.h"
#include "Icicles.h"
#include "LinesFill.h"
#include "Marquees.h"
#include "PacMan.h"
#include "Spiral.h"
#include "Strip.h"
#include "colorutils.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/printf.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/types.h"

#define NUM_STRIPS 10
#define START_PIN 2
#define STRIP_LEN 200
#define CANVAS_WIDTH 100
#define BRIGHTNESS 32
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
        //
        // First strip is a little short at the moment.
        strips[i] = new Strip(pin++, STRIP_LEN, StripType::WS2812);
        strips[i]->setColorOrder(ColorOrder::OGRB);
    }

    ArrayColorMap xmasColors({
        RGB::Red,
        RGB::Green,
        RGB::Blue,
        RGB::White,
        RGB(213, 181, 52),  // Gold
        RGB::Purple,
        RGB::Orange,
        RGB::Silver,
        RGB(255, 20, 20),    // Bright Red
        RGB(28, 191, 38),    // Vibrant Green
        RGB(0, 33, 111),     // Navy
        RGB(255, 252, 245),  // Warm White
        RGB::Yellow,
        RGB(242, 18, 18),  // Deep Red
        RGB::FairyLightNCC,
    });

    uint8_t rColors[] = {0};
    uint8_t gColors[] = {1};
    uint8_t rgColors[] = {0, 1};
    uint8_t rwColors[] = {0, 3};
    uint8_t rgwColors[] = {0, 1, 3};
    uint8_t rgbwColors[] = {0, 1, 2, 3};
    uint8_t rgbwgColors[] = {0, 1, 2, 3, 4};

    Canvas canvas(CANVAS_WIDTH);
    canvas.setBrightness(BRIGHTNESS);
    for (int i = 0; i < ns; i++) {
        canvas.add(strips[i]);
    }
    canvas.setup();
    canvas.clear();
    canvas.show();

    Animator animator(&canvas, FPS);

    // Marquees fancyMarq(&canvas, &xmasColors, 5, rgbwgColors, 20, RIGHT,
    //                    canvas.getHeight());
    // fancyMarq.setName("FMarq");
    // fancyMarq.setFPS(40);

    // animator.addTimed(&fancyMarq, 2000);

    // ArrayColorMap icicleMap(8);
    // Icicles icicles(&canvas, &icicleMap, 6, 6, RGB(0, 255, 0));
    // icicles.setName("Icicles");
    // animator.addTimed(&icicles, 2000);
    ColorBars cb2(&canvas, &xmasColors, 15, 3);
    cb2.setName("CB2");
    animator.addTimed(&cb2, 10000);

    animator.init();

    while (true) {
        animator.step();
        if (animator.getFrameCount() % 200 == 0) {
            animator.printStats();
        }
    }
}
