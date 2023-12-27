#include <stdlib.h>

#include "Animator.h"
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
    Strip strips[] = {Strip(2, 138), Strip(3, 138), Strip(4, 138)};
    int ns = 3;
    canvas.add(strips[0]);
    canvas.add(strips[1]);
    canvas.add(strips[2]);
    int delay = 100;

    //
    // Init to clear the strips and show they're working without relying on
    // rendering.
    for (int i = 0; i < ns; i++) {
        strips[i].fill(RGB::Green);
        strips[i].show();
    }
    for (int i = 0; i < ns; i++) {
        sleep_ms(delay);
        strips[i].fill(RGB::Black, 0, 138);
        strips[i].show();

        sleep_ms(delay);
        strips[i].fill(RGB::Black, 138, 276);
        strips[i].show();

        sleep_ms(delay);
        strips[i].fill(RGB::Black, 276, 414);
        strips[i].show();

        sleep_ms(delay);
        strips[i].fill(RGB::Black, 414, 552);
        strips[i].show();
    }

    Bouncer b[] = {
        Bouncer(&canvas, RGB::Red, -1, -1),
        Bouncer(&canvas, RGB::Yellow, 20, -1),
        Bouncer(&canvas, RGB::GhostWhite, 50, -1),
        Bouncer(&canvas, RGB::Green, canvas.getWidth() / 2, -1),
        Bouncer(&canvas, RGB::Blue, canvas.getWidth() - 2, -1)};

    Bouncers bouncers(&canvas, b, 5);

    Animator animator(&canvas, 10);
    animator.add(&bouncers);
    animator.init();

    int n = 0;

    while (1) {
        animator.step();
        n++;
        if (n % 500 == 0) {
            printf(
                "%d frames run, %.2f "
                "us/frame, %.2f us/show %d missed frames\n",
                n,
                animator.getAverageFrameTimeUS(), 
                animator.getAverageShowTimeUS(),
                animator.getMissedFrames());
        }
    }
}
