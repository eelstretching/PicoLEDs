#include <stdlib.h>

#include <vector>
#include "Animator.h"
#include "Fireworks.h"
#include "PacMan.h"
#include "Ghost.h"
#include "Canvas.h"
#include "Strip.h"
#include "TextAnimation.h"
#include "View.h"
#include "Xpm.h"
#include "colorutils.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "icons.h"
#include "pico/printf.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/types.h"

int main() {
    stdio_init_all();

    //
    // A canvas and a view made out of strips.
    Canvas canvas(138, 16);
    View view(138);
    Strip strips[] = {Strip(2, 552), Strip(3, 552), Strip(4, 552),
                      Strip(5, 552)};
    view.add(strips[0]);
    view.add(strips[1]);
    view.add(strips[2]);
    view.add(strips[3]);
    canvas.setView(&view, 0, 0);
    int delay = 30;

    //
    // Init to clear the strips and show they're working without relying on
    // rendering.
    for (int i = 0; i < 4; i++) {
        strips[i].fill(RGB::Green);
        strips[i].show();
    }
    for (int i = 0; i < 4; i++) {
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

    PacMan pac(&canvas, 0, 1);
    pac.setDirection(RIGHT);
    Ghost inky(&canvas, inkyColor, canvas.getWidth() - 1, 1);
    inky.setDirection(LEFT);
    Ghost blinky(inky, blinkyColor, 0, 1);
    blinky.setDirection(RIGHT);
    Ghost pinky(inky, pinkyColor, canvas.getWidth() - 1, 1);
    pinky.setDirection(LEFT);
    Ghost clyde(inky, clydeColor, 0, 1);
    clyde.setDirection(RIGHT);

    Animator animator(&canvas);

    animator.add(&pac);
    animator.add(&inky);
    animator.add(&blinky);
    animator.add(&pinky);
    animator.add(&clyde);

    animator.init();

    int n = 0;
    while (1) {
        animator.step();
        n++;
        if (n % 200 == 0) {
            printf(
                "%d frames run, %.2f "
                "us/frame, %d missed frames\n",
                n,
                animator.getAverageFrameTimeUS(), animator.getMissedFrames());
        }
    }
}
