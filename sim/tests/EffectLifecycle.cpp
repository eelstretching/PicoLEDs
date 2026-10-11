// Builds every animation in the library, runs it for a while, deletes it, and
// checks that it gave back every byte it took from the heap. This is what lets
// a show make its animations when it needs them and delete them afterwards,
// rather than building them all up front, without slowly running the Pico out
// of memory.
//
//   cmake -S sim -B build-sim
//   cmake --build build-sim --target EffectLifecycle
//   ./build-sim/EffectLifecycle
//
// It prints how much heap each animation uses while it's alive, which is a
// rough guide to what it costs on the Pico (pointers are 8 bytes here and 4
// there, so the Pico numbers are a bit smaller). It exits non-zero if anything
// leaked. Where the compiler has AddressSanitizer, the build turns it on too,
// which also catches double frees and use after free.

#include <stdio.h>
#include <stdlib.h>

#include <functional>
#include <new>
#include <vector>

#include "Animator.h"
#include "ArrayColorMap.h"
#include "BarberPole.h"
#include "Bouncer.h"
#include "Bursts.h"
#include "Canvas.h"
#include "ColorBars.h"
#include "ColorCone.h"
#include "FadeColorMap.h"
#include "FadingBars.h"
#include "Fire.h"
#include "Firework.h"
#include "FireworkWipe.h"
#include "Fireworks.h"
#include "Fireworks2D.h"
#include "FontTwoP.h"
#include "Icicles.h"
#include "LinesFill.h"
#include "Marquees.h"
#include "PacChase.h"
#include "PacWipe.h"
#include "RainbowWipe.h"
#include "RandomAnimator.h"
#include "RandomText.h"
#include "RotRandColumns.h"
#include "RotRandRows.h"
#include "RotatingColumns.h"
#include "RotatingRows.h"
#include "ScrollTexts.h"
#include "ScrollWipe.h"
#include "SimpleFont.h"
#include "Spiral.h"
#include "Strip.h"
#include "TextAnimation.h"
#include "TwinkleFox.h"
#include "XmasTree.h"

//
// Heap accounting. Every new and delete in the program goes through these, so
// we always know how many bytes are live.

static size_t liveBytes = 0;

// Room in front of each block to remember its size, kept big enough that the
// block itself stays aligned for anything.
static const size_t HEADER = alignof(std::max_align_t);

void* operator new(size_t size) {
    char* p = (char*)malloc(size + HEADER);
    if (p == nullptr) {
        throw std::bad_alloc();
    }
    *(size_t*)p = size;
    liveBytes += size;
    return p + HEADER;
}

void* operator new[](size_t size) { return operator new(size); }

void operator delete(void* ptr) noexcept {
    if (ptr == nullptr) {
        return;
    }
    char* p = (char*)ptr - HEADER;
    liveBytes -= *(size_t*)p;
    free(p);
}

void operator delete[](void* ptr) noexcept { operator delete(ptr); }
void operator delete(void* ptr, size_t) noexcept { operator delete(ptr); }
void operator delete[](void* ptr, size_t) noexcept { operator delete(ptr); }

//
// The simulator's hooks into the library. We never show anything, so these
// do as little as they can.

#include "sim_hooks.h"

static uint64_t fakeClock = 0;

uint64_t time_us_64(void) { return fakeClock; }
void sleep_us(uint64_t us) { fakeClock += us; }
uint32_t get_rand_32(void) { return (uint32_t)rand(); }
void picoleds_sim_canvas_show(Canvas*) {}
Canvas* picoleds_sim_canvas_for(Renderer*) { return nullptr; }
void picoleds_sim_frame(int, int, const uint8_t*, uint64_t, bool) {}
// colorutils.cpp's 2D blurs want one of these, though nothing here blurs.
uint16_t XY(uint8_t x, uint8_t y) { return 0; }

//
// The test itself.

static int failures = 0;

/// @brief Makes an animation, runs it for a few hundred steps, deletes it,
/// and checks that the heap is back where it started. Does that a few times
/// over, since some animations only allocate on their second init().
static void check(const char* name, std::function<Animation*()> make) {
    size_t before = liveBytes;
    size_t peak = 0;
    for (int round = 0; round < 3; round++) {
        Animation* a = make();
        for (int pass = 0; pass < 2; pass++) {
            a->init();
            for (int i = 0; i < 300; i++) {
                fakeClock += 33000;
                if (!a->step()) {
                    a->finish();
                    a->init();
                }
            }
            a->finish();
        }
        peak = MAX(peak, liveBytes - before);
        delete a;
        if (liveBytes != before) {
            printf("LEAK  %-16s %zu bytes not freed\n", name, liveBytes - before);
            failures++;
            return;
        }
    }
    printf("ok    %-16s %6zu bytes while running\n", name, peak);
}

int main() {
    //
    // So the last line before a crash makes it out.
    setvbuf(stdout, nullptr, _IONBF, 0);

    //
    // A canvas like the MegaTree's: 100 pixels wide, made from strips that zig
    // zag back and forth, 40 rows in all.
    const int width = 100;
    const int nStrips = 20;
    std::vector<Strip*> strips;
    Canvas* canvas = new Canvas(width);
    for (int i = 0; i < nStrips; i++) {
        strips.push_back(new Strip(i, 2 * width));
        canvas->add(strips.back());
    }
    canvas->setup();

    ArrayColorMap colors({RGB::Red, RGB::Green, RGB::Blue, RGB::White, RGB(213, 181, 52),
                          RGB::Purple, RGB::Orange, RGB::Silver});
    static uint8_t colorIndices[] = {0, 1, 2, 3, 4};
    SimpleFont font(FontTwoPData);

    //
    // Anything an animation keeps from here on is counted against it.

    check("BarberPole", [&] { return new BarberPole(canvas, &colors, 4); });
    check("Bouncer", [&] { return new Bouncer(canvas, &colors, RGB::Red, 3, 3); });
    check("Bursts", [&] { return new Bursts(canvas, RGB::Black, 6, RGB::White, 5); });
    check("ColorBars", [&] { return new ColorBars(canvas, &colors, 5, 4); });
    check("ColorCone", [&] { return new ColorCone(canvas, &colors); });
    check("FadingBars", [&] { return new FadingBars(canvas, &colors, 10, 4); });
    check("Fire", [&] { return new Fire(canvas, &colors, 3); });
    check("Firework", [&] { return new Firework(canvas, &colors, 2); });
    check("Fireworks", [&] { return new Fireworks(canvas, &colors); });
    check("FireworkWipe", [&] { return new FireworkWipe(canvas, &colors); });
    check("Fireworks2D", [&] { return new Fireworks2D(canvas); });
    //
    // Icicles fills in the color map it's given, so give it a fresh one each
    // time.
    ArrayColorMap icicleMap(8);
    check("Icicles", [&] {
        icicleMap = ArrayColorMap(8);
        return new Icicles(canvas, &icicleMap, 10, 6, RGB(128, 128, 128));
    });
    check("LinesFill", [&] { return new LinesFill(canvas, &colors, 5, colorIndices, UP, 1); });
    check("Marquees", [&] { return new Marquees(canvas, &colors, 5, colorIndices, 20, RIGHT); });
    check("PacChase", [&] { return new PacChase(canvas); });
    check("PacWipe", [&] { return new PacWipe(canvas, &colors); });
    check("RainbowWipe", [&] { return new RainbowWipe(canvas, &colors); });
    check("RotRandColumns", [&] { return new RotRandColumns(canvas, &colors, 4); });
    check("RotRandRows", [&] { return new RotRandRows(canvas, &colors, 4); });
    check("RotatingColumns",
          [&] { return new RotatingColumns(canvas, &colors, 5, colorIndices, 4); });
    check("RotatingRows", [&] { return new RotatingRows(canvas, &colors, 5, colorIndices, 4); });
    check("ScrollWipe", [&] { return new ScrollWipe(canvas, &colors, RIGHT); });
    check("Spiral", [&] { return new Spiral(canvas, &colors, 0, 0, 5, colorIndices, 4, 20); });
    check("TwinkleFox", [&] { return new TwinkleFox(canvas); });
    check("XmasTree", [&] { return new XmasTree(canvas, 9, 18); });

    //
    // Animations that hold other animations (or text) that you hand them.
    // Those still belong to you, so these only check the container.
    ScrollText st(canvas, &font, "MERRY CHRISTMAS", 90, -30, RGB::Red);
    check("ScrollTexts", [&] {
        ScrollTexts* sts = new ScrollTexts(canvas);
        sts->add(&st);
        return sts;
    });
    TextElement te("HO HO HO", 10, 8, RGB::Gold);
    check("TextAnimation", [&] {
        TextAnimation* ta = new TextAnimation(canvas, &font);
        ta->add(&te);
        return ta;
    });
    check("RandomText", [&] {
        RandomText* rt = new RandomText(canvas, &font);
        rt->add(&te);
        return rt;
    });
    RainbowWipe rw(canvas, &colors);
    check("MultiAnimation", [&] {
        MultiAnimation* ma = new MultiAnimation(canvas, &colors);
        ma->add(&rw);
        return ma;
    });

    //
    // Fireworks that share their fireworks with a wipe, the way the MegaTree
    // sets them up. The wipe borrows them, so it mustn't delete them.
    {
        size_t before = liveBytes;
        Fireworks* fw = new Fireworks(canvas, &colors);
        FireworkWipe* wipe =
            new FireworkWipe(canvas, &colors, fw->getFireworks(), fw->getNumFireworks());
        wipe->init();
        for (int i = 0; i < 300 && wipe->step(); i++) {
        }
        delete wipe;
        delete fw;
        if (liveBytes != before) {
            printf("LEAK  %-16s %zu bytes not freed\n", "shared wipe", liveBytes - before);
            failures++;
        } else {
            printf("ok    %-16s\n", "shared wipe");
        }
    }

    //
    // A PacChase that borrows its sprites from a PacWipe.
    {
        size_t before = liveBytes;
        PacWipe* wipe = new PacWipe(canvas, &colors);
        PacChase* chase = new PacChase(wipe);
        chase->init();
        for (int i = 0; i < 300; i++) {
            chase->step();
        }
        delete chase;
        delete wipe;
        if (liveBytes != before) {
            printf("LEAK  %-16s %zu bytes not freed\n", "borrowed chase", liveBytes - before);
            failures++;
        } else {
            printf("ok    %-16s\n", "borrowed chase");
        }
    }

    //
    // Color maps, including copying and assigning them.
    {
        size_t before = liveBytes;
        ColorMap* fade = new FadeColorMap(&colors, 4, 32, 28);
        delete fade;
        ArrayColorMap* a = new ArrayColorMap(colors);
        *a = colors;
        delete a;
        if (liveBytes != before) {
            printf("LEAK  %-16s %zu bytes not freed\n", "color maps", liveBytes - before);
            failures++;
        } else {
            printf("ok    %-16s\n", "color maps");
        }
    }

    //
    // An animator deletes the timers that addTimed wraps animations in.
    {
        size_t before = liveBytes;
        RainbowWipe* wipe = new RainbowWipe(canvas, &colors);
        RandomAnimator* animator = new RandomAnimator(canvas, 30);
        animator->addTimed(wipe, 1000);
        animator->add(wipe);
        animator->init();
        for (int i = 0; i < 100; i++) {
            animator->step();
        }
        delete animator;
        delete wipe;
        if (liveBytes != before) {
            printf("LEAK  %-16s %zu bytes not freed\n", "Animator", liveBytes - before);
            failures++;
        } else {
            printf("ok    %-16s\n", "Animator");
        }
    }

    //
    // And the canvas and strips themselves.
    {
        delete canvas;
        for (Strip* s : strips) {
            delete s;
        }
    }

    if (failures > 0) {
        printf("\n%d leaked\n", failures);
        return 1;
    }
    printf("\nNo leaks\n");
    return 0;
}
