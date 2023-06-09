#include "Canvas.h"
#include "Fire.h"
#include "Strip.h"
#include "View.h"
#include "pico/printf.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/types.h"

int main() {
    stdio_init_all();

    //
    // A canvas and a view made out of strips.
    Canvas canvas(138, 8);
    View view(138);
    Strip strips[] = {Strip(2, 276), Strip(3, 276), Strip(4, 276),
                      Strip(5, 276)};
    view.add(strips[0]);
    view.add(strips[1]);
    view.add(strips[2]);
    view.add(strips[3]);
    canvas.setView(&view, 0, 0);

    int delay = 30;

    int msPerFrame = 1000/30;

    //
    // Init to clear the strips and show they're working while rendering's
    // busted.
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
    }

    sleep_ms(delay);
    canvas.clear();
    StopWatch sw;

    Fire fire1(&canvas, 1, 20, 80, 55, 120);
    Fire fire2(&canvas, 3, 20, 80, 55, 120);
    Fire fire3(&canvas, 5, 20, 80, 55, 120);
    int n = 0;
    while (1) {
        sw.start();
        fire1.step();
        fire2.step();
        fire3.step();
        canvas.show();
        sw.finish();
        uint64_t lms = sw.getLastTimeMS();
        if(lms < msPerFrame) {
            sleep_ms(msPerFrame - lms);
        }
        n++;
        if(n % 100 == 0) {
            printf("Stepped %d times, %.2f ms/iteration\n", n, sw.getAverageTime() / 1000);
        }
    }
}