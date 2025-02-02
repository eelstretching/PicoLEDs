#include <stdlib.h>

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

#define NUM_LEDS 120
int main() {
    stdio_init_all();

    //
    // A canvas and a view made out of strips.
    Canvas canvas(NUM_LEDS);
    Strip strips[] = {Strip(2, NUM_LEDS), Strip(3, NUM_LEDS), Strip(4, NUM_LEDS)};
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
        strips[i].fill(RGB::Black, 0, NUM_LEDS);
        strips[i].show();

        sleep_ms(delay);
        strips[i].fill(RGB::Black, NUM_LEDS, NUM_LEDS * 2);
        strips[i].show();

        sleep_ms(delay);
        strips[i].fill(RGB::Black, NUM_LEDS * 2, NUM_LEDS * 3);
        strips[i].show();

        sleep_ms(delay);
        strips[i].fill(RGB::Black, NUM_LEDS * 3, NUM_LEDS * 4);
        strips[i].show();
    }

    int x = -1;
    int y = -1;
    bool up = true;
    bool right = true;

    while (1) {
        // printf("Prev %d,%d\n", x, y);
        canvas.set(x, y, canvas.getBackground());
        if(up) {
            y++;
            if(y == canvas.getHeight()-1) {
                up = false;
            }
        }  else {
            y--;
            if(y == 0) {
                up = true;
            }
        }

        if(right) {
            x++;
            if(x == canvas.getWidth() - 1) {
                right = false;
            }
        } else {
            x--;
            if(x == 0) {
                right = true;
            }
        }

        printf("Curr %d,%d\n", x, y);
        canvas.set(x, y, RGB::Green);
        canvas.show();

        sleep_ms(200);

    }

}
