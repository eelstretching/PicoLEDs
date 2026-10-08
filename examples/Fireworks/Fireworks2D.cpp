//
// Fireworks on a 2D canvas: 32 strips of 200 pixels, folded into a 100 x 64
// grid, like a lawn full of pixels. Prints how long each step takes, so we
// can see how many sparks the Pico can keep up with.
#include "Canvas.h"
#include "Fireworks2D.h"
#include "Strip.h"
#include "pico/printf.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/types.h"

#define NUM_STRIPS 32
#define START_PIN 0
#define STRIP_LEN 200
#define CANVAS_WIDTH 100
#define BRIGHTNESS 32
#define FPS 30

int main() {
    stdio_init_all();

    Canvas canvas(CANVAS_WIDTH);
    for (int i = 0; i < NUM_STRIPS; i++) {
        canvas.add(new Strip(START_PIN + i, STRIP_LEN));
    }
    canvas.setBrightness(BRIGHTNESS);
    canvas.setDithering(true);
    canvas.setup();
    canvas.clear();

    Fireworks2D fireworks(&canvas);
    fireworks.init();

    StopWatch aw;
    StopWatch sw;
    int msPerFrame = 1000 / FPS;
    int n = 0;
    while (1) {
        //
        // A finale for ten seconds out of every minute.
        fireworks.setFinale(n % (60 * FPS) >= 50 * FPS);
        aw.start();
        fireworks.step();
        aw.finish();
        sw.start();
        canvas.show();
        sw.finish();
        uint64_t ms = aw.getLastTimeMS() + sw.getLastTimeMS();
        if (ms < msPerFrame) {
            sleep_ms(msPerFrame - ms);
        }
        n++;
        if (n % 300 == 0) {
            printf("Stepped %d times, %.2f avg animation ms %.2f avg show ms, "
                   "%d sparks\n",
                   n, aw.getAverageTime() / 1000, sw.getAverageTime() / 1000,
                   fireworks.getSparkCount());
        }
    }
}
