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

int main() {
    stdio_init_all();

    //
    // A canvas made out of strips.
    Canvas canvas(138);
    Strip strips[] = {Strip(2, 552),
                      Strip(3, 552)};  // , Strip(3, 276)}; //,Strip(4, 138)};
    int ns = 2;
    for (int i = 0; i < ns; i++) {
        canvas.add(strips[i]);
    }
    int delay = 100;

    RGB colors[] = {RGB::Red,  RGB::Orange, RGB::Yellow, RGB::Green,
                    RGB::Blue, RGB::Indigo, RGB::Violet};
    int nColors = 7;

    canvas.clear();

    Bouncer bouncer(&canvas, RGB::Green, 0, 0);
    BarberPole bp(&canvas, colors, nColors, 5);

    Animator animator(&canvas, 5);
    animator.add(&bp);
    animator.init();

    int n = 0;

    while (1) {
        animator.step();
        n++;
        if (n % 500 == 0) {
            printf(
                "%d frames run, %.2f "
                "us/frame, %.2f us/show %d missed frames\n",
                n, animator.getAverageFrameTimeUS(),
                animator.getAverageShowTimeUS(), animator.getMissedFrames());
        }
    }
}
