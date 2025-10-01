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

ColorMap colorMap({RGB::Red, RGB::Green, RGB::Blue, RGB::Yellow,
                       RGB::Purple, RGB::GhostWhite, RGB::DarkViolet,
                       RGB::FireBrick});

void run_contig_test(Strip **strips, int startStrip, int endStrip, int ns,
                     int startPin) {
    printf("Testing contiguous strips %d to %d\n", strips[startStrip]->getPin(),
           strips[endStrip]->getPin());
    Renderer r;
    for (int i = startStrip; i < endStrip; i++) {
        r.add(*strips[i]);
    }
    r.setup();
    for (int i = startStrip; i < endStrip; i++) {
        strips[i]->fill(((startPin + i) % 3));
    }
    r.render(&colorMap);
    sleep_ms(100);

    for (int i = startStrip; i < endStrip; i++) {
        for (int j = 0; j < strips[i]->getNumPixels(); j++) {
            strips[i]->putPixel((j % 3), j);
        }
    }
    r.render(&colorMap);
    sleep_ms(100);

    //
    // Blank the strips.
    for (int i = startStrip; i < endStrip; i++) {
        strips[i]->fill(255);
    }
    r.render(&colorMap);
    sleep_ms(100);
}

void run_discontig_test(Strip **strips, int ns, int *s, int *e, int ds) {
    printf("Testing discontiguous strips: ");
    for (int i = 0; i < ds; i++) {
        printf("%d %d, ", s[i], e[i]);
    }
    printf("\n");
    Renderer r;

    for (int i = 0; i < ds; i++) {
        for (int j = s[i]; j < e[i]; j++) {
            r.add(*strips[j]);
        }
    }
    r.setup();

    for (int i = 0; i < ds; i++) {
        for (int j = s[i]; j < e[i]; j++) {
            strips[j]->fill((j % 3));
        }
    }
    r.render(&colorMap);
    sleep_ms(100);

    for (int i = 0; i < ds; i++) {
        for (int j = s[i]; j < e[i]; j++) {
            for (int k = 0; k < strips[j]->getNumPixels(); k++) {
                strips[j]->putPixel((k % 3), k);
            }
        }
    }
    r.render(&colorMap);
    sleep_ms(100);

    //
    // Blank the strips.
    for (int i = 0; i < ds; i++) {
        for (int j = s[i]; j < e[i]; j++) {
            strips[j]->fill(255);
        }
    }
    r.render(&colorMap);
    sleep_ms(100);
}

int main() {
    stdio_init_all();

    //
    // Testing the renderer in a variety of settings, with a variety of
    // numbers of strips.
    Strip *strips[NUM_STRIPS];
    int ns = NUM_STRIPS;
    int pin = START_PIN;
    int pins[NUM_STRIPS];
    for (int i = 0; i < ns; i++) {
        pins[i] = pin;
        strips[i] = new Strip(pin++, STRIP_LEN);
    }

    colorMap.dim(210);

    while (1) {
        // //
        // // Continugous sets of strips, which will make a single PIO program.
        for (int s = 1; s < 12; s++) {
            for (int i = 0; i < ns - (s - 1); i++) {
                run_contig_test(strips, i, i + s, ns, pins[i]);
            }
        }

        {
            int start[2];
            int end[2];
            //
            // Two discontiguous sets of size two through five.
            for (int size = 2; size < 5; size++) {
                for (int gap = 1; gap < 4; gap++) {
                    int ts = 2 * size + gap;
                    for (int i = 0; i < ns - (ts - 1); i++) {
                        start[0] = i;
                        end[0] = i + size;
                        start[1] = end[0] + gap;
                        end[1] = start[1] + size;
                        run_discontig_test(strips, ns, start, end, 2);
                    }
                }
            }
        } 
        {
            int start[3];
            int end[3];
            //
            // Three discontiguous sets of size two through five.
            for (int size = 2; size < 5; size++) {
                for (int gap = 1; gap < 5 && (3 * size + 2 * gap) < ns;
                gap++) {
                    int ts = 3 * size + 2 * gap;
                    for (int i = 0; i < ns - (ts - 1); i++) {
                        start[0] = i;
                        end[0] = i + size;
                        start[1] = end[0] + gap;
                        end[1] = start[1] + size;
                        start[2] = end[1] + gap;
                        end[2] = start[2] + size;
                        run_discontig_test(strips, ns, start, end, 3);
                    }
                }
            }
        }
    }
}
