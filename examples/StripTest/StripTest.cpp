#include <Renderer.h>
#include <stdlib.h>

#include "StopWatch.h"
#include "Strip.h"
#include "colorutils.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/printf.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/types.h"

#define STRIP_LEN 137
#define NUM_STRIPS 16
#define START_PIN 2

int main() {
    stdio_init_all();

    //
    // Simple test for a few strips of pixels.
    Strip* strips[NUM_STRIPS];
    Renderer renderer;
    int ns = NUM_STRIPS;
    int pin = START_PIN;
    for (int i = 0; i < ns; i++) {
        strips[i] = new Strip(pin++, STRIP_LEN);
        renderer.add(*strips[i]);
    }
    renderer.setup();

    ColorMap colorMap({RGB::Black, RGB::Red, RGB::Green, RGB::Blue, RGB::Yellow,
                       RGB::Purple, RGB::GhostWhite, RGB::DarkViolet,
                       RGB::FireBrick});

    printf("Ready\n");
    for (int i = 0; i < ns; i++) {
        strips[i]->fill(i % 3);
        renderer.render(&colorMap);
        sleep_ms(500);
    }

    int dirs[NUM_STRIPS];
    for (int i = 0; i < NUM_STRIPS; i++) {
        //
        // When I originally wrote the code, the even directions were 1, so
        // there you go.
        dirs[i] = (i + 1) % 2;
    }

    float onesec = 1e6;  // 1 second in microseconds
    float fps = 30;
    int frame_time_us = (int)(onesec / fps);

    printf("frame_time_us %d\n", frame_time_us);

    int width = 10;

    int posns[NUM_STRIPS];
    for (int i = 0; i < NUM_STRIPS; i++) {
        posns[i] = i % 2 == 0 ? 0 : ((int)strips[0]->getNumPixels() - width);
    }

    //
    // Blank the strips.
    for (int j = 0; j < ns; j++) {
        strips[j]->fill(RGB::Black);
    }
    renderer.render(&colorMap);

    StopWatch fw;
    while (1) {
        fw.start();
        uint64_t start = time_us_64();
        for (int s = 0; s < ns; s++) {
            if (dirs[s] == 1) {
                if (posns[s] > 0) {
                    strips[s]->putPixel(RGB::Black, posns[s] - 1);
                }
                for (int i = posns[s]; i < posns[s] + width; i++) {
                    strips[s]->putPixel(s+1, i);
                }
                posns[s]++;
                if (posns[s] == strips[s]->getNumPixels() - width) {
                    dirs[s] = 0;
                }
            } else {
                if (posns[s] < strips[s]->getNumPixels() - width) {
                    strips[s]->putPixel(RGB::Black, posns[s] + width);
                }
                for (int i = posns[s]; i < posns[s] + width; i++) {
                    strips[s]->putPixel(s+1, i);
                }
                posns[s]--;
                if (posns[s] == -1) {
                    dirs[s] = 1;
                    posns[s] = 0;
                }
            }
        }
        renderer.render(&colorMap);
        uint64_t dthis = frame_time_us - (time_us_64() - start);
        if (dthis > 0) {
            sleep_us(dthis);
        }
        fw.finish();

        if (fw.count % 200 == 0) {
            printf("%d frames, %.2f f/s\n", fw.count,
                   fw.count / (fw.totalTime / 1e6));
            printf("%d blocked\n", renderer.getBlockedCount());
            printf("%.2f us data setup time\n",
                   (double)renderer.getDataSetupTime() / fw.count);
            printf("%.2f us per DMA\n",
                   (double)renderer.getDMATime() / fw.count);
            // for (int i = 0; i < ns; i++) {
            //     StopWatch sw = strips[i]->getStripStats();
            //     printf("%d %.2fus/show\n", i, sw.getAverageTime());
            // }
        }
    }
}
