//
// Bench bring-up for a bank (output) board: a Pico 2 W drives the board's 8
// outputs over the 16-pin ribbon, with a pattern that tells the outputs apart
// so we can check the ribbon's pin mapping and every PTC and terminal.
//
// Wiring (Pico 2 W GPIO -> ribbon pin -> bank board output):
//
//   GP2 (pin 4)  -> ribbon 2  -> output 1, red
//   GP3 (pin 5)  -> ribbon 4  -> output 2, green
//   GP4 (pin 6)  -> ribbon 6  -> output 3, blue
//   GP5 (pin 7)  -> ribbon 8  -> output 4, yellow
//   GP6 (pin 9)  -> ribbon 10 -> output 5, cyan
//   GP7 (pin 10) -> ribbon 12 -> output 6, magenta
//   GP8 (pin 11) -> ribbon 14 -> output 7, white
//   GP9 (pin 12) -> ribbon 16 -> output 8, orange
//
// Ribbon odd pins 1-11 go to Pico ground, 13 to the Pico's 3V3 (pin 36) and
// 15 to 5 V (VBUS, pin 40, when the Pico is on USB). GP0/GP1 stay free for
// the UART.
//
// The pattern repeats forever:
//
// 1. Count: every output fills with its color and blinks as many times as its
//    output number (output 1 blinks once, output 8 eight times), then a pause.
//    Twice through. A wrong color on every output means the color order is
//    wrong; the right color on the wrong terminal means a ribbon or shifter
//    wiring mix-up.
// 2. Chase: a short run of pixels in each output's color travels from the
//    start of the strand to the end. Where it stops is the first bad pixel.
// 3. All on: every pixel on every output a dim white, to see the far end of
//    each strand.
//
// The Pico's own LED blinks twice a second while the pattern runs, and the
// phase is printed on USB serial and UART0.
#include "Renderer.h"
#include "Strip.h"
#include "pico/cyw43_arch.h"
#include "pico/printf.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/types.h"

#define NUM_OUTPUTS 8
#define START_PIN 2
//
// Long enough for a MegaTree strand; shorter strands just ignore the extra.
#define STRIP_LEN 200
//
// Out of 255. 16 keeps a 200 pixel strand to a few watts at full white, gentle
// for a first power-up. Raise it once everything checks out.
#define BRIGHTNESS 16
#define CHASE_LEN 10
#define CHASE_PIXELS_PER_SEC 100
#define FRAME_MS 20

static const RGB colors[NUM_OUTPUTS] = {
    RGB(255, 0, 0),    RGB(0, 255, 0),   RGB(0, 0, 255),   RGB(255, 255, 0),
    RGB(0, 255, 255),  RGB(255, 0, 255), RGB(255, 255, 255), RGB(255, 96, 0),
};

static const RGB black(0, 0, 0);

static Strip* strips[NUM_OUTPUTS];
static Renderer renderer(BRIGHTNESS);
static bool ledOn = false;
static absolute_time_t nextBlink;

//
// Sends the current frame, toggling the Pico's LED as a heartbeat.
static void show() {
    renderer.render();
    if (time_reached(nextBlink)) {
        ledOn = !ledOn;
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, ledOn);
        nextBlink = make_timeout_time_ms(250);
    }
}

//
// Holds the current frame for a while, resending it every frame so a glitch
// on the data line doesn't stick.
static void hold(uint32_t ms) {
    absolute_time_t end = make_timeout_time_ms(ms);
    while (!time_reached(end)) {
        show();
        sleep_ms(FRAME_MS);
    }
}

static void count() {
    printf("Count: output N blinks N times\n");
    for (int blink = 1; blink <= NUM_OUTPUTS; blink++) {
        for (int i = 0; i < NUM_OUTPUTS; i++) {
            strips[i]->fill(blink <= i + 1 ? colors[i] : black);
        }
        hold(400);
        for (int i = 0; i < NUM_OUTPUTS; i++) {
            strips[i]->fill(black);
        }
        hold(400);
    }
    hold(2000);
}

static void chase() {
    printf("Chase: start to end of each strand\n");
    int frames = (STRIP_LEN + CHASE_LEN) * 1000 / CHASE_PIXELS_PER_SEC / FRAME_MS;
    for (int f = 0; f <= frames; f++) {
        int head = f * FRAME_MS * CHASE_PIXELS_PER_SEC / 1000;
        for (int i = 0; i < NUM_OUTPUTS; i++) {
            strips[i]->fill(black);
            for (int p = head - CHASE_LEN + 1; p <= head; p++) {
                if (p >= 0 && p < STRIP_LEN) {
                    strips[i]->putPixel(colors[i], p);
                }
            }
        }
        show();
        sleep_ms(FRAME_MS);
    }
}

static void allOn() {
    printf("All on: dim white\n");
    for (int i = 0; i < NUM_OUTPUTS; i++) {
        strips[i]->fill(RGB(255, 255, 255));
    }
    hold(5000);
    for (int i = 0; i < NUM_OUTPUTS; i++) {
        strips[i]->fill(black);
    }
    hold(1000);
}

int main() {
    stdio_init_all();
    //
    // The cyw43 driver claims its PIO state machine first, so the renderer
    // can't take it.
    if (cyw43_arch_init()) {
        printf("cyw43 init failed, no heartbeat LED\n");
    }
    nextBlink = get_absolute_time();

    for (int i = 0; i < NUM_OUTPUTS; i++) {
        strips[i] = new Strip(START_PIN + i, STRIP_LEN);
        strips[i]->setColorOrder(ColorOrder::ORGB);
        renderer.add(strips[i]);
    }
    renderer.setup();

    printf("BankTest: %d outputs on GP%d-GP%d, %d pixels, brightness %d\n",
           NUM_OUTPUTS, START_PIN, START_PIN + NUM_OUTPUTS - 1, STRIP_LEN,
           BRIGHTNESS);

    while (true) {
        count();
        count();
        chase();
        allOn();
    }
}
