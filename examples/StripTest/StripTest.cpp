#include <Renderer.h>
#include <stdlib.h>

#include "ArrayColorMap.h"
#include "StopWatch.h"
#include "Strip.h"
#include "colorutils.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/printf.h"
#include "pico/stdio.h"
#include "pico/stdio_usb.h"
#include "pico/stdlib.h"
#include "pico/types.h"

//
// Any of these can be overridden from CMake. On the LED processor board
// (PICO_BOARD=led_processor_board) the default is all 32 outputs, GPIO 0-31
// (J5-J8), with seed pixels' red-green-blue color order.
#ifndef STRIP_LEN
#define STRIP_LEN 200
#endif
#ifdef LED_PROCESSOR_BOARD
#ifndef NUM_STRIPS
#define NUM_STRIPS 32
#endif
#ifndef START_PIN
#define START_PIN 0
#endif
#ifndef COLOR_ORDER
#define COLOR_ORDER ColorOrder::ORGB
#endif
#else
#ifndef NUM_STRIPS
#define NUM_STRIPS 10
#endif
#ifndef START_PIN
#define START_PIN 2
#endif
#endif
#define PANEL_WIDTH 10

int main() {
    stdio_init_all();
    //
    // Give the laptop a moment to open the USB serial port so the banner
    // isn't lost.
    for (int i = 0; i < 30 && !stdio_usb_connected(); i++) {
        sleep_ms(100);
    }
    printf("StripTest: %d strips on GP%d-GP%d, %d pixels each\n", NUM_STRIPS,
           START_PIN, START_PIN + NUM_STRIPS - 1, STRIP_LEN);

    //
    // Simple test for a few strips of pixels.
    Strip* strips[NUM_STRIPS];
    Renderer renderer(16);
    int ns = NUM_STRIPS;
    int pin = START_PIN;
    for (int i = 0; i < ns; i++) {
        strips[i] = new Strip(pin++, STRIP_LEN);
#ifdef COLOR_ORDER
        strips[i]->setColorOrder(COLOR_ORDER);
#endif
        renderer.add(strips[i]);
    }
    renderer.setup();

    ArrayColorMap colorMap({RGB::Red, RGB::Orange, RGB::Yellow, RGB::Green,
                            RGB::Blue, RGB::Indigo, RGB::Violet, RGB::White,
                            RGB::Gold});

    for (int c = 0; c < colorMap.getUsed(); c++) {
        for (int i = 0; i < ns; i++) {
            strips[i]->fill(colorMap.getColor(c));
        }
        renderer.render();
        sleep_ms(250);
    }
    for (int i = 0; i < ns; i++) {
        strips[i]->fill(colorMap.getBackground());
    }
    renderer.render();
    sleep_ms(100);

    //
    // Fill with color bands.
    for (int s = 0; s < ns; s++) {
        Strip& strip = *strips[s];
        uint8_t cc = 0;
        for (int i = 0; i < strip.getNumPixels(); i++) {
            strip.putPixel(colorMap.getColor(cc), i);
            if ((i + 1) % PANEL_WIDTH == 0) {
                cc = (cc + 1) % colorMap.getUsed();
            }
        }
    }
    renderer.render();
    sleep_ms(1000);

    float fps = 30;
    float usPerFrame = 1e6 / fps;
    StopWatch frameWatch;
    uint32_t missedFrames = 0;
    int startPos = 0;
    int width = PANEL_WIDTH;
    int currColorIndex = 0;
    while (1) {
        frameWatch.start();
        for (int s = 0; s < ns; s++) {
            if (s % 2 == 0) {
                strips[s]->rotate(LEFT);
            } else {
                strips[s]->rotate(RIGHT);
            }   
        }

        renderer.render();
        frameWatch.finish();
        uint64_t lus = frameWatch.getLastTime();
        if (lus < usPerFrame) {
            sleep_us(usPerFrame - lus);
        } else {
            missedFrames++;
        }

        if (frameWatch.count % 200 == 0) {
            float avg = frameWatch.getAverageTime();
            printf("%u frames, %.2f us/frame budget, %.2f us frame time, "
                   "%.1f fps max, %u missed ",
                   frameWatch.count, usPerFrame, avg, 1e6 / avg,
                   missedFrames);

            printf("%d blocked ", renderer.getBlockedCount());
            printf("%.2f us per DMA\n",
                   (double)renderer.getDMATime() / frameWatch.count);
        }
    }

}
