#include "Animator.h"
#include "Canvas.h"
#include "Fire.h"
#include "Strip.h"
#include "pico/printf.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/types.h"
#include <ArrayColorMap.h>

int main() {
    stdio_init_all();

    //
    // A canvas and a view made out of strips.
    Canvas canvas(138);
    Strip strips[] = {Strip(2, 138), Strip(3, 138), Strip(4, 138)};
    int ns = 3;
    for(int i = 0; i < ns; i++) {
        canvas.add(strips[i]);
    }

    int delay = 30;
    sleep_ms(delay);
    canvas.clear();
    StopWatch sw;

    ArrayColorMap colorMap({
        RGB::Black, RGB::Red,  RGB::Orange, RGB::Yellow, RGB::Green,
        RGB::Blue,  RGB::Indigo, RGB::Violet
    });
    colorMap.setBrightness(32);

    int nf = 3;
    Fire f[] = {Fire(&canvas, &colorMap, 10, 80, 0, 55, 120),
                Fire(&canvas, &colorMap, 10, 100, 1, 80, 150),
                Fire(&canvas, &colorMap, 10, 90, 2, 30, 120)};

    MultiAnimation fires(&canvas, &colorMap);
    for (int i = 0; i < nf; i++) {
        fires.add(&f[i]);
    }

    Animator animator(&canvas, &colorMap, 5);
    animator.add(&fires);
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