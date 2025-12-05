#include <Bursts.h>
#include <ColorBars.h>
#include <FadingBars.h>
#include <RandomAnimation.h>
#include <RotRandColumns.h>
#include <RotRandRows.h>
#include <RotatingColumns.h>
#include <RotatingRows.h>
#include <TimedAnimation.h>
#include <stdlib.h>

#include <string>

#include "Animator.h"
#include "ArrayColorMap.h"
#include "BarberPole.h"
#include "Bouncer.h"
#include "Canvas.h"
#include "ColorCone.h"
#include "Icicles.h"
#include "LinesFill.h"
#include "Marquees.h"
#include "Spiral.h"
#include "Strip.h"
#include "colorutils.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/printf.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/types.h"

#define NUM_STRIPS 5
#define START_PIN 2
#define STRIP_LEN 400
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

    ArrayColorMap basicXmasColors({RGB::Red, RGB::White, RGB::Green});

    basicXmasColors.setBrightness(32);

    ArrayColorMap dimXmasColors(xmasColors);
    dimXmasColors.setBrightness(16);

    ArrayColorMap midXmasColors(xmasColors);
    midXmasColors.setBrightness(32);

    ArrayColorMap brightXmasColors(xmasColors);
    brightXmasColors.setBrightness(128);

    Canvas canvas(CANVAS_WIDTH);
    for (int i = 0; i < ns; i++) {
        canvas.add(*strips[i]);
    }
    canvas.setup();
    canvas.setColorMap(&basicXmasColors);

    for (int i = 0; i < basicXmasColors.getUsed(); i++) {
        canvas.fill(i);
        canvas.show();
        sleep_ms(500);
    }

    canvas.clear();
    canvas.show();

    Animator animator(&canvas, &midXmasColors, FPS);

    RotRandColumns rc1(&canvas, &dimXmasColors, 2);
    rc1.setName("RRC1");
    animator.addTimed(&rc1, 10000);

    RotRandColumns rc2(&canvas, &dimXmasColors, 4);
    rc1.setName("RRC2");
    animator.addTimed(&rc2, 10000);

    // Marquees marq(&canvas, &dimXmasColors, midXmasColors.getSize(), 20,
    // RIGHT,
    //               canvas.getHeight());
    // marq.setName("Marquee");
    // animator.add(&marq);

    RotRandRows rr1(&canvas, &dimXmasColors, 4);
    rr1.setName("RRR1");
    animator.addTimed(&rr1, 10000);

    RotRandRows rr2(&canvas, &dimXmasColors, 2);
    rr2.setName("RRR2");
    animator.addTimed(&rr2, 10000);

    animator.init();

    while (true) {
        animator.step();
        if (animator.getFrameCount() % 200 == 0) {
            animator.printStats();
        }
    }
}
