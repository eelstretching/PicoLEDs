#include <stdlib.h>

#include "Animator.h"
#include "BarberPole.h"
#include "Bouncer.h"
#include "Canvas.h"
#include "Strip.h"
#include "colorutils.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/printf.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/types.h"
#include <ArrayColorMap.h>

#define NUM_STRIPS 16
#define START_PIN 2
#define STRIP_LEN 137

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
        strips[i] = new Strip(pin++, STRIP_LEN);
    }

    //
    // A bit pattern that slowly increases the number of leading
    // zeros on the colors that we're feeding into the LEDS
    Canvas canvas(STRIP_LEN);
    for (int i = 0; i < ns; i++) {
        canvas.add(*strips[i]);
    }
    canvas.setup();
    ArrayColorMap colorMap({RGB(0b11111111, 0b11111111, 0b11111111),
                       RGB(0b11111111, 0b00111111, 0b11111111),
                       RGB(0b11111111, 0b00011111, 0b11111111),
                       RGB(0b11111111, 0b00001111, 0b11111111),
                       RGB(0b11111111, 0b00000111, 0b11111111),
                       RGB(0b11111111, 0b00000011, 0b11111111),
                       RGB(0b11111111, 0b00000001, 0b11111111),
                       RGB(0b11111111, 0b00000000, 0b11111111),
                       RGB(0b01111111, 0b00000000, 0b11111111),
                       RGB(0b00111111, 0b00000000, 0b11111111),
                       RGB(0b00011111, 0b00000000, 0b11111111),
                       RGB(0b00001111, 0b00000000, 0b11111111),
                       RGB(0b00000111, 0b00000000, 0b11111111),
                       RGB(0b00000011, 0b00000000, 0b11111111),
                       RGB(0b00000001, 0b00000000, 0b11111111),
                       RGB(0b00000000, 0b00000000, 0b11111111),
                       RGB(0b00000000, 0b00000000, 0b01111111),
                       RGB(0b00000000, 0b00000000, 0b00111111),
                       RGB(0b00000000, 0b00000000, 0b00011111),
                       RGB(0b00000000, 0b00000000, 0b00001111),
                       RGB(0b00000000, 0b00000000, 0b00000111),
                       RGB(0b00000000, 0b00000000, 0b00000011),
                       RGB(0b00000000, 0b00000000, 0b00000001),
                       RGB(0b00000000, 0b00000000, 0b00000000)});
    canvas.setColorMap(&colorMap);
    canvas.clear();
    canvas.show();
    int nColors = colorMap.getUsed();

    for (int i = 0; i < nColors; i++) {
        printf("Setting %");
        colorMap.setBackground(colorMap.getColor(i));
        canvas.clear();
        canvas.show();
        sleep_ms(1000);
    }
    colorMap.setBackground(colorMap.getColor(0));
    canvas.clear();
    canvas.show();

    BarberPole bp(&canvas, &colorMap, nColors, 10);

    Animator animator(&canvas, &colorMap, 5);
    animator.add(&bp);
    animator.init();
    animator.setFPS(30);

    int n = 0;

    while (1) {
        animator.step();
        if (animator.getFrameCount() % 1000 == 0) {
            printf(
                "%d frames run, %.2f us/frame at %d fps %.2f us/step, %.2f "
                "us/show %.2f us/frame %d missed frames\n",
                animator.getFrameCount(), animator.getUsPerFrame(),
                animator.getFPSNeeded(), animator.getAverageStepTimeUS(),
                animator.getAverageShowTimeUS(),
                animator.getAverageFrameTimeUS(), animator.getMissedFrames());
        }
    }
}
