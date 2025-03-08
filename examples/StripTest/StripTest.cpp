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
#define NUM_STRIPS 1
#define START_PIN 2

int main() {
    stdio_init_all();

    //
    // Simple test for a single strip of pixels.
    Strip *strips[NUM_STRIPS];
    Renderer renderer;
    int ns = NUM_STRIPS;
    int pin = START_PIN;
    for (int i = 0; i < ns; i++) {
        strips[i] = new Strip(pin++, STRIP_LEN);
        strips[i]->setFractionalBrightness(32);
        renderer.add(*strips[i]);
    }
    renderer.setup();

    RGB colors[] = {RGB::Red,        RGB::Green,    RGB::Blue,
                    RGB::Yellow,     RGB::Purple,   RGB::GhostWhite,
                    RGB::DarkViolet, RGB::FireBrick};

    float fps = 10;
    int delay = (int)(1000 / fps);

    int dirs[NUM_STRIPS];
    for (int i = 0; i < NUM_STRIPS; i++) {
        //
        // When I originally wrote the code, the even directions were 1, so
        // there you go.
        dirs[i] = (i + 1) % 2;
    }

    int width = 9;

    int posns[NUM_STRIPS];
    for (int i = 0; i < NUM_STRIPS; i++) {
        posns[i] = i % 2 == 0 ? 0 : ((int)strips[0]->getNumPixels() - width);
    }

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < ns; j++) {
            strips[j]->fill(RGB::Red);
        }
        renderer.render();
        sleep_ms(250);
        for (int j = 0; j < ns; j++) {
            strips[j]->fill(RGB::Green);
        }
        renderer.render();
        sleep_ms(250);
        for (int j = 0; j < ns; j++) {
            strips[j]->fill(RGB::Blue);
        }
        renderer.render();
        sleep_ms(250);
        for (int j = 0; j < ns; j++) {
            strips[j]->fill(RGB::Black);
        }
        renderer.render();
        sleep_ms(250);
    }

    StopWatch fw;
    while (1) {
        fw.start();
        for (int s = 0; s < ns; s++) {
            if (dirs[s] == 1) {
                if (posns[s] > 0) {
                    strips[s]->putPixel(RGB::Black, posns[s] - 1);
                }
                for (int i = posns[s]; i < posns[s] + width; i++) {
                    strips[s]->putPixel(colors[s], i);
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
                    strips[s]->putPixel(colors[s], i);
                }
                posns[s]--;
                if (posns[s] == -1) {
                    dirs[s] = 1;
                    posns[s] = 0;
                }
            }
        }
        renderer.render();
        if (delay > 0) {
            sleep_ms(delay);
        }

        fw.finish();
        if (fw.count % 1000 == 0) {
            printf("%d frames, %.2f f/s\n", fw.count,
                   fw.count / (fw.totalTime / 1e6));
            printf("%d blocked\n", renderer.getBlockedCount());
            printf("%.2f us per DMA\n",
                   (double)renderer.getDMATime() / fw.count);
            // for (int i = 0; i < ns; i++) {
            //     StopWatch sw = strips[i]->getStripStats();
            //     printf("%d %.2fus/show\n", i, sw.getAverageTime());
            // }
        }
    }
}
