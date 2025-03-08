#include <stdlib.h>

#include "Animator.h"
#include "Canvas.h"
#include "Font.h"
#include "FontTwoP.h"
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
    // A canvas and a view made out of strips.
    Canvas canvas(138);
    Strip strips[] = {Strip(2, 552), Strip(3, 552), Strip(4, 552),
                      Strip(5, 552)};
    canvas.add(strips[0]);
    canvas.add(strips[1]);
    canvas.add(strips[2]);
    canvas.add(strips[3]);
    int delay = 30;

    Font twoP(&canvas, FontTwoPData);

    //
    // A couple of animations.
    while (1) {
        int w = twoP.render("prepared", 10, 8, RGB::ForestGreen);
        canvas.show();

        printf("Render was %d pixels wide\n", w);
        sleep_ms(1000);
    }
}
