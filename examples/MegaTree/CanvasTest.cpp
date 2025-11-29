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
    for (int i = 0; i < ns; i++, pin++) {
        pins[i] = pin;
        //
        // First strip is a little short at the moment.
        strips[i] = new Strip(pin, i == 0 ? (STRIP_LEN-100) : STRIP_LEN);
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

    for(int i = 0; i < c.getHeight(); i++ ) {
        c.fillRow(i, i % dimXmasColors.getUsed());
    }
    c.show();
    sleep_ms(2000);

    int nr = 0;
    while (true) {
            c.rotateUp();
            c.show();
            sleep_ms(250);
            nr++;
            if(nr % 10 == 0) {
                printf("Rotated %d times\n", nr);
            }
    }
}
