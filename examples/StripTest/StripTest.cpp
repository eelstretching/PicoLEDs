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
//
// STRIP_LEN is how many pixels each strip lights at boot. Type a number and
// press Enter on the USB serial port (tio) to light that many instead, up to
// MAX_PIXELS (a whole 1,000-pixel roll); 0 turns everything off. Pixels past
// the count are sent black, so shrinking it really turns the tail off.
#ifndef STRIP_LEN
#define STRIP_LEN 200
#endif
#ifndef MAX_PIXELS
#define MAX_PIXELS 1000
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


static Strip* strips[NUM_STRIPS];
static Renderer* renderer = nullptr;
//
// How many pixels each strip lights, and how long the strips are: the most
// ever lit, so pixels that were lit once and aren't now keep getting black.
static int lit = 0;
static int stripLen = 0;

static ArrayColorMap colorMap({RGB::Red, RGB::Orange, RGB::Yellow, RGB::Green,
                               RGB::Blue, RGB::Indigo, RGB::Violet, RGB::White,
                               RGB::Gold});

//
// Throws away the strips and renderer and makes new ones n pixels long. Only
// done when the count grows past anything sent so far.
static void makeStrips(int n) {
    delete renderer;
    for (int i = 0; i < NUM_STRIPS; i++) {
        delete strips[i];
    }
    renderer = new Renderer(16);
    for (int i = 0; i < NUM_STRIPS; i++) {
        strips[i] = new Strip(START_PIN + i, n);
#ifdef COLOR_ORDER
        strips[i]->setColorOrder(COLOR_ORDER);
#endif
        renderer->add(strips[i]);
    }
    renderer->setup();
    stripLen = n;
}

//
// Lights the first n pixels of every strip with color bands, PANEL_WIDTH
// pixels each, and blacks out the rest.
static void fillBands(int n) {
    if (n > stripLen) {
        makeStrips(n);
    }
    for (int s = 0; s < NUM_STRIPS; s++) {
        Strip& strip = *strips[s];
        strip.fill(RGB::Black);
        uint8_t cc = 0;
        for (int i = 0; i < n; i++) {
            strip.putPixel(colorMap.getColor(cc), i);
            if ((i + 1) % PANEL_WIDTH == 0) {
                cc = (cc + 1) % colorMap.getUsed();
            }
        }
    }
    lit = n;
}

static void printStatus() {
    printf("\n>>> LIT %d px per strip on GP%d-GP%d\n", lit, START_PIN,
           START_PIN + NUM_STRIPS - 1);
    printf("type a number 0-%d + Enter: ", MAX_PIXELS);
}

//
// Digits typed so far, waiting for Enter.
static char line[8];
static int lineLen = 0;

//
// Reads whatever serial input is waiting, without blocking. Digits collect
// into a number, echoed so it shows in tio; Enter (CR or LF) relights with
// it; backspace deletes; anything else is ignored. Returns the new count when
// one was entered, otherwise -1.
static int pollSerial() {
    int c;
    while ((c = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT) {
        if (c >= '0' && c <= '9') {
            if (lineLen < (int)sizeof(line) - 1) {
                line[lineLen++] = (char)c;
                putchar(c);
            }
        } else if ((c == '\b' || c == 127) && lineLen > 0) {
            lineLen--;
            printf("\b \b");
        } else if (c == '\r' || c == '\n') {
            if (lineLen == 0) {
                continue;
            }
            line[lineLen] = 0;
            lineLen = 0;
            int n = atoi(line);
            if (n > MAX_PIXELS) {
                printf("\n!!! %d is more than %d, lighting %d", n, MAX_PIXELS,
                       MAX_PIXELS);
                n = MAX_PIXELS;
            }
            return n;
        }
    }
    return -1;
}

int main() {
    stdio_init_all();
    //
    // Give the laptop a moment to open the USB serial port so the banner
    // isn't lost.
    for (int i = 0; i < 30 && !stdio_usb_connected(); i++) {
        sleep_ms(100);
    }
    int bootLen = STRIP_LEN > MAX_PIXELS ? MAX_PIXELS : STRIP_LEN;
    printf("StripTest: %d strips on GP%d-GP%d, %d pixels each\n", NUM_STRIPS,
           START_PIN, START_PIN + NUM_STRIPS - 1, bootLen);

    //
    // Simple test for a few strips of pixels.
    int ns = NUM_STRIPS;
    makeStrips(bootLen);

    for (int c = 0; c < colorMap.getUsed(); c++) {
        for (int i = 0; i < ns; i++) {
            strips[i]->fill(colorMap.getColor(c));
        }
        renderer->render();
        sleep_ms(250);
    }
    for (int i = 0; i < ns; i++) {
        strips[i]->fill(colorMap.getBackground());
    }
    renderer->render();
    sleep_ms(100);

    //
    // Fill with color bands.
    fillBands(bootLen);
    renderer->render();
    sleep_ms(1000);
    printStatus();

    float fps = 30;
    float usPerFrame = 1e6 / fps;
    StopWatch frameWatch;
    uint32_t missedFrames = 0;
    while (1) {
        int n = pollSerial();
        if (n >= 0) {
            fillBands(n);
            printStatus();
            //
            // A new renderer starts its stats over, so the frame stats do too.
            frameWatch = StopWatch();
            missedFrames = 0;
        }

        frameWatch.start();
        if (lit > 1) {
            for (int s = 0; s < ns; s++) {
                if (s % 2 == 0) {
                    strips[s]->rotateLeft(0, lit);
                } else {
                    strips[s]->rotateRight(0, lit);
                }
            }
        }

        renderer->render();
        frameWatch.finish();
        uint64_t lus = frameWatch.getLastTime();
        if (lus < usPerFrame) {
            sleep_us(usPerFrame - lus);
        } else {
            missedFrames++;
        }

        //
        // The stats wait while a number is half typed, so they don't break
        // up the echo.
        if (frameWatch.count % 200 == 0 && lineLen == 0) {
            float avg = frameWatch.getAverageTime();
            printf("%u frames, %d px, %.2f us/frame budget, %.2f us frame "
                   "time, %.1f fps max, %u missed ",
                   frameWatch.count, lit, usPerFrame, avg, 1e6 / avg,
                   missedFrames);

            printf("%d blocked ", renderer->getBlockedCount());
            printf("%.2f us per DMA\n",
                   (double)renderer->getDMATime() / frameWatch.count);
        }
    }
}
