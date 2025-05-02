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

#define NUM_STRIPS 16
#define START_PIN 2
#define STRIP_LEN 137

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
        strips[i]->setFractionalBrightness(32);
    }

    RGB colors[] = {RGB::Red,  RGB::Orange, RGB::Yellow, RGB::Green,
                    RGB::Blue, RGB::Indigo, RGB::Violet};
    int nColors = 7;

    while (1) {
        for (int sc = 4; sc <= 12; sc++) {
            for (int i = 0; i < ns - (sc - 1); i++) {
                Canvas canvas(STRIP_LEN);
                for (int j = 0; j < sc; j++) {
                    canvas.add(*strips[i + j]);
                }
                canvas.setup();
                
                BarberPole bp(&canvas, colors, nColors, 5);

                Animator animator(&canvas);
                animator.add(&bp);
                animator.init();
                animator.setFPS(60);
                while (animator.getFrameCount() < 200) {
                    animator.step();
                }
                canvas.clear();
                canvas.show();
                printf(
                    "%d frames run, %.2f us/frame at %d fps used %.2f "
                    "us/frame, "
                    "%.2f "
                    "us/show %d missed frames\n",
                    animator.getFrameCount(), animator.getUsPerFrame(),
                    animator.getFPSNeeded(), animator.getAverageFrameTimeUS(),
                    animator.getAverageShowTimeUS(),
                    animator.getMissedFrames());
            }
        }
    }
}
