// Makes and deletes strips, renderers, canvases, animators and animations over
// and over, the way a show that builds its effects on demand would, and checks
// that every cycle gives back all the heap, DMA channels and PIO state machines
// it took. Like RenderTest, but for everything rather than just the Renderer.
//
// Each cycle picks one of three things to do, at random:
//
//  - one canvas: a random set of strips on GPIO 2-9 (so some runs are parallel
//    and some are single), a random canvas shape, dithering on or off, and an
//    animator running one to three random animations for a couple of seconds.
//  - two canvases at once, on separate pins. The first one is deleted while
//    the second keeps rendering, which only works if deleting a renderer leaves
//    the shared DMA interrupt handler alone for the other one.
//  - bare renderers, made, rendered and deleted at once, many times over, so
//    they're torn down while their DMA is still sending.
//
// After each cycle it prints a line with the heap in use, and complains if the
// heap, DMA channels or state machines aren't back where they were after the
// first few cycles. It doesn't need any LEDs connected, but it's more fun with some
// on GPIO 2-9. If a cycle hangs, the watchdog reboots the Pico and the next
// boot says so.

#include <malloc.h>
#include <stdio.h>

#include <algorithm>
#include <functional>
#include <vector>

#include "Animator.h"
#include "ArrayColorMap.h"
#include "BarberPole.h"
#include "Bouncer.h"
#include "Bursts.h"
#include "Canvas.h"
#include "ColorBars.h"
#include "ColorCone.h"
#include "FadingBars.h"
#include "Fire.h"
#include "FireworkWipe.h"
#include "Fireworks.h"
#include "Fireworks2D.h"
#include "Icicles.h"
#include "LinesFill.h"
#include "Marquees.h"
#include "PacChase.h"
#include "PacWipe.h"
#include "RainbowWipe.h"
#include "RandomAnimator.h"
#include "Renderer.h"
#include "RotRandColumns.h"
#include "RotRandRows.h"
#include "RotatingColumns.h"
#include "RotatingRows.h"
#include "ScrollWipe.h"
#include "Spiral.h"
#include "Strip.h"
#include "TwinkleFox.h"
#include "XmasTree.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/watchdog.h"
#include "math8.h"
#include "pico/stdlib.h"

#define START_PIN 2
#define NUM_PINS 8

/// @brief How long a cycle can go without checking in before the watchdog
/// decides it's hung.
#define WATCHDOG_MS 8000

/// @brief How many cycles to run before we start checking for leaks.
#define WARMUP_CYCLES 3

static ArrayColorMap colors({RGB::Red, RGB::Green, RGB::Blue, RGB::White,
                             RGB(213, 181, 52), RGB::Purple, RGB::Orange,
                             RGB::Silver});
static uint8_t colorIndices[] = {0, 1, 2, 3, 4};

/// @brief An animation we made, and the color map we made for it, if any.
struct Made {
    Animation* animation;
    ColorMap* extra;
};

struct Effect {
    const char* name;
    std::function<Made(Canvas*)> make;
};

static std::vector<Effect> effects = {
    {"BarberPole", [](Canvas* c) { return Made{new BarberPole(c, &colors, 4), nullptr}; }},
    {"Bouncer", [](Canvas* c) { return Made{new Bouncer(c, &colors, RGB::Red, 3, 0), nullptr}; }},
    {"Bursts", [](Canvas* c) { return Made{new Bursts(c, RGB::Black, 6, RGB::White, 5), nullptr}; }},
    {"ColorBars", [](Canvas* c) { return Made{new ColorBars(c, &colors, 5, 4), nullptr}; }},
    {"ColorCone", [](Canvas* c) { return Made{new ColorCone(c, &colors), nullptr}; }},
    {"FadingBars", [](Canvas* c) { return Made{new FadingBars(c, &colors, 10, 4), nullptr}; }},
    {"Fire", [](Canvas* c) { return Made{new Fire(c, &colors, 0), nullptr}; }},
    {"Fireworks", [](Canvas* c) { return Made{new Fireworks(c, &colors), nullptr}; }},
    {"FireworkWipe", [](Canvas* c) { return Made{new FireworkWipe(c, &colors), nullptr}; }},
    {"Fireworks2D", [](Canvas* c) { return Made{new Fireworks2D(c), nullptr}; }},
    {"Icicles",
     [](Canvas* c) {
         //
         // Icicles fills in the color map it's given, so it gets its own.
         ArrayColorMap* map = new ArrayColorMap(8);
         return Made{new Icicles(c, map, MIN(10, c->getHeight()), 6, RGB(128, 128, 128)), map};
     }},
    {"LinesFill", [](Canvas* c) { return Made{new LinesFill(c, &colors, 5, colorIndices, UP, 1), nullptr}; }},
    {"Marquees", [](Canvas* c) { return Made{new Marquees(c, &colors, 5, colorIndices, 20, RIGHT), nullptr}; }},
    {"PacChase", [](Canvas* c) { return Made{new PacChase(c), nullptr}; }},
    {"PacWipe", [](Canvas* c) { return Made{new PacWipe(c, &colors), nullptr}; }},
    {"RainbowWipe", [](Canvas* c) { return Made{new RainbowWipe(c, &colors), nullptr}; }},
    {"RotRandColumns", [](Canvas* c) { return Made{new RotRandColumns(c, &colors, 4), nullptr}; }},
    {"RotRandRows", [](Canvas* c) { return Made{new RotRandRows(c, &colors, 4), nullptr}; }},
    {"RotatingColumns", [](Canvas* c) { return Made{new RotatingColumns(c, &colors, 5, colorIndices, 4), nullptr}; }},
    {"RotatingRows", [](Canvas* c) { return Made{new RotatingRows(c, &colors, 5, colorIndices, 4), nullptr}; }},
    {"ScrollWipe", [](Canvas* c) { return Made{new ScrollWipe(c, &colors, RIGHT), nullptr}; }},
    {"Spiral", [](Canvas* c) { return Made{new Spiral(c, &colors, 0, 0, 5, colorIndices, 4, 20), nullptr}; }},
    {"TwinkleFox", [](Canvas* c) { return Made{new TwinkleFox(c), nullptr}; }},
    {"XmasTree", [](Canvas* c) { return Made{new XmasTree(c, 9, 18), nullptr}; }},
};

static void freeMade(Made& m) {
    delete m.animation;
    delete m.extra;
}

//
// What we watch for leaks.

struct Resources {
    size_t heap;
    int dmaChannels;
    int stateMachines;
    int pioPrograms;
};

static Resources measure() {
    Resources r;
    r.heap = mallinfo().uordblks;
    r.dmaChannels = 0;
    for (uint i = 0; i < NUM_DMA_CHANNELS; i++) {
        r.dmaChannels += dma_channel_is_claimed(i) ? 1 : 0;
    }
    r.stateMachines = 0;
    for (uint p = 0; p < NUM_PIOS; p++) {
        for (uint sm = 0; sm < 4; sm++) {
            r.stateMachines += pio_sm_is_claimed(pio_get_instance(p), sm) ? 1 : 0;
        }
    }
    r.pioPrograms = 0;
    for (int i = 0; i < NUM_DMA_CHANNELS; i++) {
        r.pioPrograms += pioPrograms[i] != nullptr ? 1 : 0;
    }
    return r;
}

/// @brief Picks n different pins from GPIO START_PIN to START_PIN + NUM_PINS
/// - 1, leaving out any that are already taken, in increasing order.
static std::vector<uint> pickPins(int n, uint32_t& taken) {
    std::vector<uint> pins;
    while ((int)pins.size() < n) {
        uint p = random8(NUM_PINS);
        if (taken & (1u << p)) {
            continue;
        }
        taken |= 1u << p;
        pins.push_back(START_PIN + p);
    }
    std::sort(pins.begin(), pins.end());
    return pins;
}

/// @brief A canvas, its strips, and an animator running some animations on
/// it, all made from scratch.
struct Show {
    std::vector<Strip*> strips;
    Canvas* canvas;
    RandomAnimator* animator;
    std::vector<Made> made;

    Show(int nStrips, uint32_t& taken) {
        //
        // A canvas 50 or 100 wide, with each strip folded into 1, 2 or 4 rows.
        uint width = random8(2) == 0 ? 50 : 100;
        uint rowsPerStrip = 1 << random8(3);
        if (nStrips * rowsPerStrip < 4) {
            //
            // Some animations need a few rows to work with.
            rowsPerStrip = 4;
        }
        StripType type = random8(2) == 0 ? WS2812 : WS2811;
        canvas = new Canvas(width);
        for (uint pin : pickPins(nStrips, taken)) {
            strips.push_back(new Strip(pin, width * rowsPerStrip, type));
            canvas->add(strips.back());
        }
        canvas->setBrightness(random8(8, 48));
        canvas->setDithering(random8(2) == 0);
        canvas->setup();

        animator = new RandomAnimator(canvas, 30);
        int n = random8(1, 4);
        for (int i = 0; i < n; i++) {
            Effect& e = effects[random8(effects.size())];
            Made m = e.make(canvas);
            m.animation->setName(e.name);
            made.push_back(m);
            animator->addTimed(m.animation, random16(500, 1500));
        }
        animator->init();
        printf("  canvas %ux%u, %d %s strips on GPIO", width, canvas->getHeight(),
               nStrips, type == WS2812 ? "WS2812" : "WS2811");
        for (Strip* s : strips) {
            printf(" %d", s->getPin());
        }
        printf(":");
        for (Made& m : made) {
            printf(" %s", m.animation->getName());
        }
        printf("\n");
    }

    ~Show() {
        //
        // In the order a show would: the animator and animations, then the
        // canvas (and its renderer), then the strips it drew on.
        delete animator;
        for (Made& m : made) {
            freeMade(m);
        }
        delete canvas;
        for (Strip* s : strips) {
            delete s;
        }
    }

    void step() {
        animator->step();
        watchdog_update();
    }
};

static void oneCanvas() {
    uint32_t taken = 0;
    Show show(random8(1, NUM_PINS + 1), taken);
    int frames = random16(30, 90);
    for (int i = 0; i < frames; i++) {
        show.step();
    }
}

static void twoCanvases() {
    uint32_t taken = 0;
    int n1 = random8(1, NUM_PINS);
    Show* first = new Show(n1, taken);
    Show* second = new Show(random8(1, NUM_PINS - n1 + 1), taken);
    for (int i = 0; i < 45; i++) {
        first->step();
        second->step();
    }
    //
    // The second one has to keep going after the first is gone.
    delete first;
    for (int i = 0; i < 45; i++) {
        second->step();
    }
    delete second;
}

static void bareRenderers() {
    printf("  50 bare renderers, deleted while sending\n");
    for (int round = 0; round < 50; round++) {
        uint32_t taken = 0;
        std::vector<Strip*> strips;
        Renderer* r = new Renderer(random8(8, 48));
        r->setDithering(random8(2) == 0);
        uint len = random16(10, 400);
        for (uint pin : pickPins(random8(1, NUM_PINS + 1), taken)) {
            strips.push_back(new Strip(pin, len));
            strips.back()->fill(RGB(random8(), random8(), random8()));
            r->add(strips.back());
        }
        r->setup();
        int renders = random8(1, 4);
        for (int i = 0; i < renders; i++) {
            r->render();
        }
        delete r;
        for (Strip* s : strips) {
            delete s;
        }
        watchdog_update();
    }
}

int main() {
    stdio_init_all();
    sleep_ms(2000);
    printf("\nLifecycleStress\n");
    if (watchdog_caused_reboot()) {
        printf("*** The watchdog rebooted us: the last run hung partway through a cycle.\n");
    }
    watchdog_enable(WATCHDOG_MS, true);

    Resources baseline;
    uint problems = 0;
    for (uint cycle = 1;; cycle++) {
        int mode = random8(3);
        printf("Cycle %u: %s\n", cycle,
               mode == 0 ? "one canvas" : mode == 1 ? "two canvases" : "bare renderers");
        switch (mode) {
            case 0:
                oneCanvas();
                break;
            case 1:
                twoCanvases();
                break;
            case 2:
                bareRenderers();
                break;
        }
        Resources now = measure();
        if (cycle <= WARMUP_CYCLES) {
            //
            // The first few cycles set up things that stay around for good
            // (stdio's buffers, for one), so we measure from after them.
            baseline = now;
            printf("  heap in use %u bytes (warming up)\n", (uint)now.heap);
            watchdog_update();
            continue;
        }
        bool ok = true;
        if (now.heap != baseline.heap) {
            printf("  *** heap in use is %d bytes off where it started\n",
                   (int)now.heap - (int)baseline.heap);
            ok = false;
        }
        if (now.dmaChannels != baseline.dmaChannels) {
            printf("  *** %d DMA channels claimed, should be %d\n", now.dmaChannels,
                   baseline.dmaChannels);
            ok = false;
        }
        if (now.stateMachines != baseline.stateMachines) {
            printf("  *** %d PIO state machines claimed, should be %d\n", now.stateMachines,
                   baseline.stateMachines);
            ok = false;
        }
        if (now.pioPrograms != 0) {
            printf("  *** %d PIO programs still registered\n", now.pioPrograms);
            ok = false;
        }
        if (!ok) {
            problems++;
            //
            // Measure from here, so one leak is reported once, not every cycle.
            baseline = now;
        }
        printf("  heap in use %u bytes, %u problems so far\n", (uint)now.heap, problems);
        watchdog_update();
    }
}
