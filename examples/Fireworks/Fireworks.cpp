#include "Canvas.h"
#include "Firework.h"
#include "Strip.h"
#include "pico/printf.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/types.h"

int main() {
    stdio_init_all();

    //
    // A canvas and a view made out of strips.
    Canvas canvas(138);
    Strip strips[] = {Strip(2, 138), Strip(3, 138), Strip(4, 138)};
    int ns = 3;
    canvas.add(strips[0]);
    canvas.add(strips[1]);
    canvas.add(strips[2]);

    int delay = 100;

    int msPerFrame = 1000 / 60;

    //
    // Init to clear the strips and show they're working while rendering's
    // busted.
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

    sleep_ms(delay);
    canvas.clear();
    StopWatch aw;
    StopWatch sw;

    int nf = canvas.getHeight();
    Firework **fw = new Firework*[nf];
    for(int i = 0; i < nf; i++) {
        fw[i] = new Firework(&canvas, i);
    }

    int n = 0;
    while (1) {
        aw.start();
        for(int i = 0; i < nf; i++) {
            fw[i]->step();
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
        if (n % 500 == 0) {
            printf("Stepped %d times, %.2f avg animation ms %.2f avg show ms\n",
                   n, aw.getAverageTime() / 1000, sw.getAverageTime() / 1000);
            for(int i = 0; i < 4; i++) {
                printf(" Strip %d avg show time %.2f us\n", i, strips[i].getStripStats().getAverageTime());
            }
        }
    }
}