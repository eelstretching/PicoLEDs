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
#define NUM_STRIPS 8
#define START_PIN 2

int main() {
    stdio_init_all();

    //
    // Simple test for a single strip of pixels.
    Strip *strips[NUM_STRIPS];
    int ns = NUM_STRIPS;
    int pin = START_PIN;
    for(int i = 0; i < ns; i++) {
        strips[i] = new Strip(pin++, STRIP_LEN);
    }

    RGB colors[] = {RGB::Red, RGB::Green, RGB::Blue, RGB::Yellow, RGB::Purple, RGB::GhostWhite, RGB::DarkViolet, RGB::FireBrick};

    for (int i = 0; i < ns; i++) {
        strips[i]->setFractionalBrightness(16);
    }
    
    float fps = 60;
    int delay =  (int) (1000/fps);

    printf("Delay is %d\n", delay);

    int dirs[NUM_STRIPS];
    for(int i = 0; i < NUM_STRIPS; i++) {
        dirs[i] = (i+1) % 2;
    }

    for(int i = 0; i < NUM_STRIPS; i++) {
        printf("Dir %d: %d\n", i, dirs[i]);
    }

    int width = 9;

    int posns[NUM_STRIPS];
    for(int i = 0; i < NUM_STRIPS; i++) {
        posns[i] = i % 2 == 0 ? 0 : ((int) strips[0]->getNumPixels() - width);
    }

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < ns; j++) {
            strips[j]->fill(RGB::Red);
            strips[j]->show();
        }
        sleep_ms(250);
        for (int j = 0; j < ns; j++) {
            strips[j]->fill(RGB::Green);
            strips[j]->show();
        }
        sleep_ms(250);
        for (int j = 0; j < ns; j++) {
            strips[j]->fill(RGB::Blue);
            strips[j]->show();
        }
        sleep_ms(250);
        for (int j = 0; j < ns; j++) {
            strips[j]->fill(RGB::Black);
            strips[j]->show();
        }
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
            strips[s]->show();
        }
        if (delay > 0) {
            sleep_ms(delay);
        }

        fw.finish();
        if (fw.count % 500 == 0) {
            printf("%d frames, %.2f f/s\n", fw.count,
                   fw.count / (fw.totalTime / 1e6));
            printf("%d blocked\n", strips[0]->nblocked);
            printf("%.2f us per DMA\n", ((float) strips[0]->getDMATime())/fw.count);
            for (int i = 0; i < ns; i++) {
                StopWatch sw = strips[i]->getStripStats();
                printf("%d %.2fus/show\n", i, sw.getAverageTime());
            }
        }
    }
}
