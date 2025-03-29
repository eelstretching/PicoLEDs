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

#define STRIP_LEN 130
#define NUM_STRIPS 4
#define START_PIN 10

int main() {
    stdio_init_all();

    //
    // Simple test for a few strips of pixels.
    Strip *strips[NUM_STRIPS];
    Renderer renderer;
    int ns = NUM_STRIPS;
    int pin = START_PIN;
    for (int i = 0; i < ns; i++) {
        strips[i] = new Strip(pin++, STRIP_LEN);
        strips[i]->setFractionalBrightness(0x20);
        renderer.add(*strips[i]);
    }
    renderer.setup();

    RGB colors[] = {RGB::Red,        RGB::Green,    RGB::Blue,
                    RGB::Yellow,     RGB::Purple,   RGB::GhostWhite,
                    RGB::DarkViolet, RGB::FireBrick};

    RGB patterns[] = {RGB(0b10101010, 0b10101010, 0b10101010),
                      RGB(0b01010101, 0b01010101, 0b01010101),
                      RGB(0b10011001, 0b10011001, 0b10011001),
                      RGB(0b01100110, 0b01100110, 0b01100110),
                      RGB(0b00110011, 0b00110011, 0b00110011),
                      RGB(0b11001100, 0b11001100, 0b11001100),
                      RGB(0b10001000, 0b10001000, 0b10001000),
                      RGB(0b00010001, 0b00010001, 0b00010001)};

    for (int k = 0; k < 5; k++) {
        for (int i = 0; i < ns; i++) {
            strips[i]->fill(colors[k]);
        }
        renderer.render();
        sleep_ms(1000);
        for (int i = 0; i < ns; i++) {
            for (int j = 0; j < strips[i]->getNumPixels(); j++) {
                strips[i]->putPixel(colors[j % 3], j);
            }
        }
        renderer.render();
        sleep_ms(1000);
    }

    sleep_ms(2000);

    int dirs[NUM_STRIPS];
    for (int i = 0; i < NUM_STRIPS; i++) {
        //
        // When I originally wrote the code, the even directions were 1, so
        // there you go.
        dirs[i] = (i + 1) % 2;
    }

    float onesec = 1e6;  // 1 second in microseconds
    float fps = 60;
    int delay_us = (int)(onesec / fps);

    printf("delay_us %d\n", delay_us);

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
    renderer.render();
    sleep_ms(250);

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
        uint64_t dthis = delay_us - (time_us_64() - start);
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
