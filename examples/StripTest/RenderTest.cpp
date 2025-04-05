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

RGB colors[] = {RGB::Red,    RGB::Green,      RGB::Blue,       RGB::Yellow,
                RGB::Purple, RGB::GhostWhite, RGB::DarkViolet, RGB::FireBrick};

RGB patterns[] = {RGB(0b10101010, 0b10101010, 0b10101010),
                  RGB(0b01010101, 0b01010101, 0b01010101),
                  RGB(0b10011001, 0b10011001, 0b10011001),
                  RGB(0b01100110, 0b01100110, 0b01100110),
                  RGB(0b00110011, 0b00110011, 0b00110011),
                  RGB(0b11001100, 0b11001100, 0b11001100),
                  RGB(0b10001000, 0b10001000, 0b10001000),
                  RGB(0b00010001, 0b00010001, 0b00010001)};

void run_contig_test(Strip **strips, int startStrip, int endStrip, int ns,
                     int startPin) {
    printf("Testing contiguous strips %d to %d\n", startStrip, endStrip);
    Renderer r;
    for (int i = startStrip; i < endStrip; i++) {
        r.add(*strips[i]);
    }
    r.setup();
    for (int i = startStrip; i < endStrip; i++) {
        strips[i]->fill(colors[(startPin + i) % 3]);
    }
    r.render();
    sleep_ms(100);

    for (int i = startStrip; i < endStrip; i++) {
        for (int j = 0; j < strips[i]->getNumPixels(); j++) {
            strips[i]->putPixel(colors[j % 3], j);
        }
    }
    r.render();
    sleep_ms(100);

    //
    // Blank the strips.
    for (int i = startStrip; i < endStrip; i++) {
        strips[i]->fill(RGB::Black);
    }
    r.render();
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
            strips[j]->fill(colors[j % 3]);
        }
    }
    r.render();
    sleep_ms(100);

    for (int i = 0; i < ds; i++) {
        for (int j = s[i]; j < e[i]; j++) {
            for (int k = 0; k < strips[j]->getNumPixels(); k++) {
                strips[j]->putPixel(colors[k % 3], k);
            }
        }
    }
    r.render();
    sleep_ms(100);

    //
    // Blank the strips.
    for (int i = 0; i < ds; i++) {
        for (int j = s[i]; j < e[i]; j++) {
            strips[j]->fill(RGB::Black);
        }
    }
    r.render();
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
        strips[i]->setFractionalBrightness(16);
    }

    while (1) {
        // //
        // // Continugous sets of strips, which will make a single PIO program.
        for (int s = 1; s < 15; s++) {
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
                for (int gap = 1; gap < 5 && (3 * size + 2 * gap) < ns; gap++) {
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
