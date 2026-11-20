//
// One strip on any pin, with the pin and the pixel count set over the USB
// serial port (tio), for wiring up a single strand such as the tree topper on
// GP35. Shows StripTest's rotating color bands, ten pixels per band, so a
// strand can be counted off in tens.
//
// Commands, each ended with Enter:
//   p <pin>   drive the strip on GPIO <pin> instead (the old pin goes dark)
//   n <num>   light <num> pixels, 0-MAX_PIXELS; 0 turns everything off
//   <num>     same as n <num>
//   ?         show the commands
//
// Pixels past the count are sent black, so shrinking it really turns the tail
// off. USB stdio is on, so `picotool load -f -x` works while it runs.
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "ArrayColorMap.h"
#include "Renderer.h"
#include "StopWatch.h"
#include "Strip.h"
#include "hardware/gpio.h"
#include "pico/stdio.h"
#include "pico/stdio_usb.h"
#include "pico/stdlib.h"

#ifndef STRIP_LEN
#define STRIP_LEN 200
#endif
#ifndef MAX_PIXELS
#define MAX_PIXELS 1000
#endif
#ifdef LED_PROCESSOR_BOARD
//
// The topper's pin on J2.
#ifndef START_PIN
#define START_PIN 35
#endif
#ifndef COLOR_ORDER
#define COLOR_ORDER ColorOrder::ORGB
#endif
#else
#ifndef START_PIN
#define START_PIN 2
#endif
#endif
#define PANEL_WIDTH 10

static Strip* strip = nullptr;
static Renderer* renderer = nullptr;
static int pin = -1;
//
// How many pixels are lit, and how long the strip is: the most ever lit on
// this pin, so pixels that were lit once and aren't now keep getting black.
static int lit = 0;
static int stripLen = 0;

static ArrayColorMap colorMap({RGB::Red, RGB::Orange, RGB::Yellow, RGB::Green,
                               RGB::Blue, RGB::Indigo, RGB::Violet, RGB::White,
                               RGB::Gold});

//
// Why a pin can't take a strip, or nullptr when it can.
static const char* pinProblem(int p) {
    if (p < 0 || p >= (int)NUM_BANK0_GPIOS) {
        return "no such GPIO";
    }
#ifdef PICO_DEFAULT_UART_TX_PIN
    if (p == PICO_DEFAULT_UART_TX_PIN || p == PICO_DEFAULT_UART_RX_PIN) {
        return "it's the debug UART";
    }
#endif
#ifdef LED_PROCESSOR_BOARD
    if (p >= 43) {
        return "it's the radio (43-46) or PSRAM (47)";
    }
#elif defined(CYW43_WL_GPIO_LED_PIN)
    if (p == 23 || p == 24 || p == 25 || p == 29) {
        return "it's the radio";
    }
#endif
    return nullptr;
}

//
// Blacks out the current strip, lets the frame go out, and lets go of its pin.
static void dropStrip() {
    if (strip == nullptr) {
        return;
    }
    strip->fill(RGB::Black);
    renderer->render();
    //
    // 1000 pixels take 30 ms to send.
    sleep_ms(50);
    delete renderer;
    delete strip;
    renderer = nullptr;
    strip = nullptr;
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, 0);
    stripLen = 0;
}

static void makeStrip(int p, int n) {
    dropStrip();
    pin = p;
    renderer = new Renderer(16);
    strip = new Strip(pin, n);
#ifdef COLOR_ORDER
    strip->setColorOrder(COLOR_ORDER);
#endif
    renderer->add(strip);
    renderer->setup();
    stripLen = n;
}

//
// Lights the first n pixels with color bands, PANEL_WIDTH pixels each, and
// blacks out the rest.
static void fillBands(int n) {
    if (n > stripLen) {
        makeStrip(pin, n);
    }
    strip->fill(RGB::Black);
    uint8_t cc = 0;
    for (int i = 0; i < n; i++) {
        strip->putPixel(colorMap.getColor(cc), i);
        if ((i + 1) % PANEL_WIDTH == 0) {
            cc = (cc + 1) % colorMap.getUsed();
        }
    }
    lit = n;
}

static void printHelp() {
    printf("\ncommands (then Enter):\n");
    printf("  p <pin>   run on GPIO <pin>\n");
    printf("  n <num>   light <num> pixels, 0-%d (0 = off)\n", MAX_PIXELS);
    printf("  <num>     same as n <num>\n");
    printf("  ?         this list\n");
}

static void printStatus() {
    printf("\n>>> GP%d: %d px\n", pin, lit);
    printf("p <pin> / n <num> / ? + Enter: ");
}

//
// The line typed so far, waiting for Enter.
static char line[16];
static int lineLen = 0;

//
// Reads whatever serial input is waiting, without blocking, echoing it so it
// shows in tio; backspace deletes. Returns true when a whole line is in
// `line`.
static bool pollSerial() {
    int c;
    while ((c = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT) {
        if (c == '\r' || c == '\n') {
            if (lineLen == 0) {
                continue;
            }
            line[lineLen] = 0;
            lineLen = 0;
            return true;
        } else if ((c == '\b' || c == 127) && lineLen > 0) {
            lineLen--;
            printf("\b \b");
        } else if (isprint(c) && lineLen < (int)sizeof(line) - 1) {
            line[lineLen++] = (char)c;
            putchar(c);
        }
    }
    return false;
}

//
// Acts on a line. Returns false when it wasn't understood.
static bool runCommand(const char* s) {
    while (*s == ' ') s++;
    char cmd = 'n';
    if (*s == '?' || *s == 'h' || *s == 'H') {
        printHelp();
        return true;
    }
    if (isalpha((unsigned char)*s)) {
        cmd = tolower(*s++);
        while (*s == ' ') s++;
    }
    if (!isdigit((unsigned char)*s)) {
        return false;
    }
    int v = atoi(s);
    if (cmd == 'p') {
        const char* why = pinProblem(v);
        if (why != nullptr) {
            printf("\n!!! can't use GP%d: %s", v, why);
            return true;
        }
        int n = lit;
        makeStrip(v, n > 0 ? n : 1);
        fillBands(n);
        return true;
    }
    if (cmd == 'n') {
        if (v > MAX_PIXELS) {
            printf("\n!!! %d is more than %d, lighting %d", v, MAX_PIXELS,
                   MAX_PIXELS);
            v = MAX_PIXELS;
        }
        fillBands(v);
        return true;
    }
    return false;
}

int main() {
    stdio_init_all();
    for (int i = 0; i < 30 && !stdio_usb_connected(); i++) {
        sleep_ms(100);
    }
    int bootLen = STRIP_LEN > MAX_PIXELS ? MAX_PIXELS : STRIP_LEN;
    printf("PinStrip: one strip, boots on GP%d with %d pixels\n", START_PIN,
           bootLen);
    printHelp();

    makeStrip(START_PIN, bootLen);
    fillBands(bootLen);
    printStatus();

    const float usPerFrame = 1e6 / 30;
    StopWatch frameWatch;
    absolute_time_t nextStatus = make_timeout_time_ms(10000);
    while (1) {
        if (pollSerial()) {
            if (!runCommand(line)) {
                printf("\n??? didn't get \"%s\"", line);
                printHelp();
            }
            printStatus();
            nextStatus = make_timeout_time_ms(10000);
        }

        frameWatch.start();
        if (lit > 1) {
            strip->rotateLeft(0, lit);
        }
        renderer->render();
        frameWatch.finish();
        uint64_t lus = frameWatch.getLastTime();
        if (lus < usPerFrame) {
            sleep_us(usPerFrame - lus);
        }
        //
        // The reminder waits while a line is half typed, so it doesn't break
        // up the echo.
        if (lineLen == 0 && absolute_time_diff_us(get_absolute_time(),
                                                  nextStatus) <= 0) {
            printStatus();
            nextStatus = make_timeout_time_ms(10000);
        }
    }
}
