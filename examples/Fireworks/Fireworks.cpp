#include "Canvas.h"
#include "Firework.h"
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

    int msPerFrame = 1000 / 60;

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
    StopWatch aw;
    StopWatch sw;

    int nf = 8;
    Firework fw[8] = {
        Firework(&canvas, 0),
        Firework(&canvas, 1),
        Firework(&canvas, 2),
        Firework(&canvas, 3),
        Firework(&canvas, 4),
        Firework(&canvas, 5),
        Firework(&canvas, 6),
        Firework(&canvas, 7),
    };

    int n = 0;
    while (1) {
        aw.start();
        for(int i = 0; i < nf; i++) {
            fw[i].step();
        }
        aw.finish();
        sw.start();
        canvas.show();
        sw.finish();
        uint64_t lms = sw.getLastTimeMS();
        if (lms < msPerFrame) {
            sleep_ms(msPerFrame - lms);
        }
        n++;
        if (n % 100 == 0) {
            printf("Stepped %d times, %.2f avg animation ms %.2f avg show ms\n",
                   n, aw.getAverageTime() / 1000, sw.getAverageTime() / 1000);
        }
    }
}