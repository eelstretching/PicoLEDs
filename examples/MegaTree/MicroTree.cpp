#include <Bursts.h>
#include <ColorBars.h>
#include <FadingBars.h>
#include <RandomAnimator.h>
#include <RotRandColumns.h>
#include <RotRandRows.h>
#include <RotatingColumns.h>
#include <RotatingRows.h>
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
#include <FontTwoP.h>
#include <TextAnimation.h>
#include <ScrollText.h>

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

    ArrayColorMap dimXmasColors(xmasColors);
    dimXmasColors.setBrightness(16);

    ArrayColorMap midXmasColors(xmasColors);
    midXmasColors.setBrightness(64);

    ArrayColorMap brightXmasColors(xmasColors);
    brightXmasColors.setBrightness(128);

    ArrayColorMap xmasTreeColors(xmasColors);
    xmasTreeColors.setBackground(RGB::ForestGreen);
    xmasTreeColors.setBrightness(16);

    ArrayColorMap spiralColors(xmasColors);
    spiralColors.setBackground(RGB::ForestGreen);
    spiralColors.setBrightness(16);

    uint8_t rColors[] = {0};
    uint8_t gColors[] = {1};
    uint8_t rgColors[] = {0, 1};
    uint8_t rwColors[] = {0, 3};
    uint8_t rgwColors[] = {0, 1, 3};
    uint8_t rgbwColors[] = {0, 1, 2, 3};
    uint8_t rgbwgColors[] = {0, 1, 2, 3, 4};

    Canvas canvas(CANVAS_WIDTH);
    for (int i = 0; i < ns; i++) {
        canvas.add(*strips[i]);
    }
    canvas.setup();
    canvas.setColorMap(&midXmasColors);
    canvas.clear();
    canvas.show();

    Animator animator(&canvas, FPS);

    Font font(&canvas, FontTwoPData);
    ScrollText ta(&canvas, &dimXmasColors, &font);
    ta.setAngle(RenderAngle::RENDER_90);
    ta.add(new TextElement("  MERRY", 30, 0, 0));
    ta.add(new TextElement("CHRISTMAS", 20, 0, 1));
    
    animator.addTimed(&ta, 20000);
    
    animator.init();

    while (true) {
        animator.step();
        if (animator.getFrameCount() % 200 == 0) {
            animator.printStats();
        }
    }
}
