#include <stdlib.h>

#include "PacWipe.h"
#include <vector>
#include "Animator.h"
#include "Fireworks.h"
#include "PacMan.h"
#include "Ghost.h"
#include "Canvas.h"
#include "Strip.h"
#include "TextAnimation.h"
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
    // A canvas made out of strips.
    Canvas canvas(138);
    Strip strips[] = {Strip(2, 552), Strip(3, 552), Strip(4, 552),
                      Strip(5, 552)};
    canvas.add(strips[0]);
    canvas.add(strips[1]);
    canvas.add(strips[2]);
    canvas.add(strips[3]);
    int delay = 30;

    PacWipe pac(&canvas);
    Animator animator(&canvas);

    animator.add(&pac);

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
