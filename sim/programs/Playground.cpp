// A place to try things out. Every .cpp in sim/programs/ becomes a program
// named after the file, so copy this one, change it, and re-run the build.
// It's an ordinary Pico program, so when it looks right it'll run on a Pico
// too.

#include "Animator.h"
#include "ArrayColorMap.h"
#include "Canvas.h"
#include "Marquees.h"
#include "Spiral.h"
#include "Strip.h"
#include "pico/stdlib.h"

#define NUM_STRIPS 32
#define STRIP_LEN 100

int main() {
    stdio_init_all();

    Canvas canvas(STRIP_LEN);
    for (int i = 0; i < NUM_STRIPS; i++) {
        canvas.add(new Strip(2 + i, STRIP_LEN));
    }
    canvas.setBrightness(32);
    canvas.setup();

    ArrayColorMap colors({RGB::Red, RGB::Green, RGB::Blue, RGB::White,
                          RGB(213, 181, 52)});
    uint8_t all[] = {0, 1, 2, 3, 4};
    uint8_t redWhite[] = {0, 3};

    Animator animator(&canvas, 30);

    Spiral spiral(&canvas, &colors, 0, 0, 4, all, 10, 25);
    animator.addTimed(&spiral, 10000);

    Marquees marquees(&canvas, &colors, 2, redWhite, 25, RIGHT,
                      canvas.getHeight());
    animator.addTimed(&marquees, 10000);

    animator.init();
    while (true) {
        animator.step();
    }
}
