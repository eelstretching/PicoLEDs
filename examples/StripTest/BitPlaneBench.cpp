//
// A microbenchmark for building the bit planes that the parallel PIO program
// sends out.
//
// Part 1 times the old bit-at-a-time loop against the 8x8 bit transpose that
// the Renderer now uses, on the same random pixel data, for every group size
// from 2 to 8 strips, and checks that they produce exactly the same bytes.
// These are standalone copies of the two loops, so they time only the bit
// planes and not the brightness scaling.
//
// Part 2 times the real Renderer on 8 strips of 200 pixels, with and without
// dithering, using the Renderer's own data setup stopwatch (what the
// animators print as us/dst). Note that this really does drive GPIO 2-9.
#include <Renderer.h>
#include <stdlib.h>
#include <string.h>

#include "Strip.h"
#include "color.h"
#include "math8.h"
#include "pico/stdlib.h"

#define STRIP_LEN 200
#define MAX_STRIPS 8
#define START_PIN 2
#define REPS 50

static uint32_t vals[MAX_STRIPS][STRIP_LEN];
static uint8_t oldBuf[STRIP_LEN * 24];
static uint8_t newBuf[STRIP_LEN * 24];

/// @brief The bit planes the way the Renderer used to build them.
static void oldPlanes(int size, uint8_t* buf) {
    memset(buf, 0, STRIP_LEN * 24);
    for (int sn = 0; sn < size; sn++) {
        uint8_t stripBit = 1 << sn;
        for (int j = 0; j < STRIP_LEN; j++) {
            uint8_t* pipbuff = &buf[j * 24];
            uint32_t val = vals[sn][j];
            for (int k = 0; k < 24; k++) {
                if (val & (0x00800000 >> k)) {
                    pipbuff[k] |= stripBit;
                }
            }
        }
    }
}

/// @brief Same as the one in Renderer.cpp.
static inline void transpose8(uint32_t& x, uint32_t& y) {
    uint32_t t;
    t = (x ^ (x >> 7)) & 0x00AA00AA;
    x = x ^ t ^ (t << 7);
    t = (y ^ (y >> 7)) & 0x00AA00AA;
    y = y ^ t ^ (t << 7);
    t = (x ^ (x >> 14)) & 0x0000CCCC;
    x = x ^ t ^ (t << 14);
    t = (y ^ (y >> 14)) & 0x0000CCCC;
    y = y ^ t ^ (t << 14);
    t = (x & 0xF0F0F0F0) | ((y >> 4) & 0x0F0F0F0F);
    y = ((x << 4) & 0xF0F0F0F0) | (y & 0x0F0F0F0F);
    x = t;
}

/// @brief The bit planes the way the Renderer builds them now.
static void newPlanes(int size, uint8_t* buf) {
    uint8_t* pb = buf;
    for (int j = 0; j < STRIP_LEN; j++, pb += 24) {
        uint32_t hi[3] = {0, 0, 0};
        uint32_t lo[3] = {0, 0, 0};
        for (int sn = 0; sn < size; sn++) {
            uint32_t val = vals[sn][j];
            uint32_t* w = sn < 4 ? lo : hi;
            int shift = 8 * (sn & 3);
            w[0] |= ((val >> 16) & 0xFF) << shift;
            w[1] |= ((val >> 8) & 0xFF) << shift;
            w[2] |= (val & 0xFF) << shift;
        }
        for (int c = 0; c < 3; c++) {
            transpose8(hi[c], lo[c]);
            uint32_t a = __builtin_bswap32(hi[c]);
            uint32_t b = __builtin_bswap32(lo[c]);
            memcpy(pb + 8 * c, &a, 4);
            memcpy(pb + 8 * c + 4, &b, 4);
        }
    }
}

static void benchPlanes() {
    for (int sn = 0; sn < MAX_STRIPS; sn++) {
        for (int j = 0; j < STRIP_LEN; j++) {
            vals[sn][j] = get_rand_32() & 0xFFFFFF;
        }
    }
    printf("Bit planes for %d pixels per strip, average of %d runs\n",
           STRIP_LEN, REPS);
    printf("strips   old us   new us  speedup  match\n");
    for (int size = 2; size <= MAX_STRIPS; size++) {
        uint64_t start = time_us_64();
        for (int r = 0; r < REPS; r++) {
            oldPlanes(size, oldBuf);
        }
        float oldUS = (time_us_64() - start) / (float)REPS;

        start = time_us_64();
        for (int r = 0; r < REPS; r++) {
            newPlanes(size, newBuf);
        }
        float newUS = (time_us_64() - start) / (float)REPS;

        bool match = memcmp(oldBuf, newBuf, sizeof(oldBuf)) == 0;
        printf("%6d %8.1f %8.1f %7.1fx  %s\n", size, oldUS, newUS,
               oldUS / newUS, match ? "yes" : "NO");
    }
}

static Renderer renderer(32);
static Strip* strips[MAX_STRIPS];

//
// How many times we've rendered, which is also how many times the Renderer's
// data setup stopwatch has run, since all 8 strips are in one group.
static uint renders = 0;

static void setupRenderer() {
    for (int i = 0; i < MAX_STRIPS; i++) {
        strips[i] = new Strip(START_PIN + i, STRIP_LEN);
        renderer.add(strips[i]);
    }
    renderer.setup();
}

static void benchRenderer() {
    for (int i = 0; i < MAX_STRIPS; i++) {
        for (int j = 0; j < STRIP_LEN; j++) {
            strips[i]->putPixel(RGB(random8(), random8(), random8()), j);
        }
    }

    printf("\nRenderer on %d strips of %d pixels (GPIO %d-%d)\n", MAX_STRIPS,
           STRIP_LEN, START_PIN, START_PIN + MAX_STRIPS - 1);
    for (int d = 0; d < 2; d++) {
        renderer.setDithering(d == 1);
        //
        // Throw away the first one, which allocates the dither remainders.
        renderer.render();
        renders++;
        //
        // The stopwatch only keeps a running average, so we'll back out the
        // average for just this batch from the totals.
        float before = renderer.getAverageDataSetupTime() * renders;
        uint64_t start = time_us_64();
        for (int r = 0; r < REPS; r++) {
            renderer.render();
        }
        float perRender = (time_us_64() - start) / (float)REPS;
        renders += REPS;
        float setup =
            (renderer.getAverageDataSetupTime() * renders - before) / REPS;
        printf("dithering %-3s: %.1f us data setup, %.1f us per render "
               "including the wait for the strip\n",
               d == 1 ? "on" : "off", setup, perRender);
    }

    //
    // Leave the strips dark.
    for (int i = 0; i < MAX_STRIPS; i++) {
        strips[i]->fill(RGB::Black);
    }
    renderer.setDithering(false);
    renderer.render();
    renders++;
}

int main() {
    stdio_init_all();
    //
    // Give a serial terminal time to connect.
    sleep_ms(3000);
    setupRenderer();

    while (true) {
        printf("\nRunning at %lu MHz\n", clock_get_hz(clk_sys) / 1000000);
        benchPlanes();
        benchRenderer();
        sleep_ms(10000);
    }
}
