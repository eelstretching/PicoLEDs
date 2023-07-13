#include <stdlib.h>

#include "Animator.h"
#include "Canvas.h"
#include "Strip.h"
#include "TextAnimation.h"
#include "View.h"
#include "icons.h"
#include "Xpm.h"
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


    printf("We are here!\n");
    Xpm sun(sun_xpm);
    Xpm rain(rain_xpm);
    printf("sun:  %d %d\n", sun.getWidth(), sun.getHeight());
    printf("rain: %d %d\n", rain.getWidth(), rain.getHeight());
    sun.render(&canvas, 10, 0);
    rain.render(&canvas, 30, 0);
    canvas.show();

    int n = 0;
    while (1) {
        printf("Running\n");
        sleep_ms(5000);
    }
}
