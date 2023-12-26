#include "Canvas.h"
#include "Fire.h"
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
    Strip strips[] = {Strip(2, 552), Strip(3, 552), Strip(4, 552),
                      Strip(5, 552)};
    canvas.add(strips[0]);
    canvas.add(strips[1]);
    canvas.add(strips[2]);
    canvas.add(strips[3]);

    int delay = 30;

    int msPerFrame = 1000 / 30;

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

    int nf = 6;
    Fire fires[6] = {Fire(&canvas, 10, 95, 1, 55, 120),
                     Fire(&canvas, 10, 100, 2, 80, 150),
                     Fire(&canvas, 10, 110, 3, 30, 120),
                     Fire(&canvas, 10, 90, 4, 20, 50),
                     Fire(&canvas, 10, 105, 5, 55, 110),
                     Fire(&canvas, 10, 100, 6, 100, 200)};

    int n = 0;
    while (1) {
        sw.start();
        for (int i = 0; i < 6; i++) {
            fires[i].step();
        }
        canvas.show();
        sw.finish();
        uint64_t lms = sw.getLastTimeMS();
        if (lms < msPerFrame) {
            sleep_ms(msPerFrame - lms);
        }
        n++;
        if (n % 100 == 0) {
            printf("Stepped %d times, %.2f ms/iteration\n", n,
                   sw.getAverageTime() / 1000);
        }
    }
}