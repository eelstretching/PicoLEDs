// Checks how the Animator paces frames and steps animations, on a pretend
// clock:
//
// - frames start on whole multiples of the frame time on the Animator's clock,
//   including a clock that's offset from time_us_64() (a shared show clock)
//   and one that jumps backwards;
// - animations with their own step rate get stepped at that rate on average,
//   whatever the frame rate;
// - clock-driven animations (fps 0) get stepped once per frame, and their
//   frame times add up to the time that passed;
// - timed animations hand the clock on to the animation they wrap;
// - a slow frame counts as missed and the next one realigns.
//
//   cmake -S sim -B build-sim
//   cmake --build build-sim --target AnimatorPacing
//   ./build-sim/AnimatorPacing

#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include "Animator.h"
#include "Canvas.h"
#include "Starlight.h"
#include "Strip.h"
#include "sim_hooks.h"

static uint64_t fakeClock = 1000003;  // deliberately not on a frame boundary

uint64_t time_us_64(void) { return fakeClock; }
void sleep_us(uint64_t us) { fakeClock += us; }
uint32_t get_rand_32(void) { return (uint32_t)rand(); }
void picoleds_sim_canvas_show(Canvas*) {}
Canvas* picoleds_sim_canvas_for(Renderer*) { return nullptr; }
void picoleds_sim_frame(int, int, const uint8_t*, uint64_t, bool) {}
uint16_t XY(uint8_t x, uint8_t y) { return 0; }

static int failures = 0;

#define CHECK(cond)                                                \
    do {                                                           \
        if (!(cond)) {                                             \
            printf("%s:%d: FAILED: %s\n", __FILE__, __LINE__, #cond); \
            failures++;                                            \
        }                                                          \
    } while (0)

/// @brief Counts its steps and remembers the frame times it was given.
class Counter : public Animation {
   public:
    int steps = 0;
    uint64_t totalFrameUs = 0;
    std::vector<uint64_t> times;
    uint64_t stepCost = 0;  // pretend each step takes this long
    Counter(Canvas* c, uint8_t fps) : Animation(c, nullptr, fps) {}
    void init() override {
        steps = 0;
        totalFrameUs = 0;
        times.clear();
    }
    bool step() override {
        steps++;
        totalFrameUs += getFrameUs();
        times.push_back(nowUs);
        fakeClock += stepCost;
        return true;
    }
    uint64_t now() const { return nowUs; }
};

/// @brief An Animator paced by a clock offset from time_us_64(), like a
/// follower's show clock.
class OffsetAnimator : public Animator {
   public:
    int64_t offset = 0;
    OffsetAnimator(Canvas* c, int fps) : Animator(c, fps) {}
    uint64_t clockUs() override { return time_us_64() + offset; }
};

static void testLegacyRates(Canvas* canvas) {
    printf("legacy step rates at 40 frames a second\n");
    for (int sfps : {10, 20, 30, 40, 55, 60, 80, 120}) {
        Animator animator(canvas, 40);
        Counter c(canvas, sfps);
        animator.add(&c);
        animator.init();
        int frames = 400;  // 10 s
        int maxPerFrame = 0;
        for (int i = 0; i < frames; i++) {
            int before = c.steps;
            animator.step();
            maxPerFrame = MAX(maxPerFrame, c.steps - before);
        }
        int expected = sfps * 10;
        printf("  %3d fps: %4d steps in 10 s (want %d), at most %d per frame\n", sfps, c.steps,
               expected, maxPerFrame);
        CHECK(abs(c.steps - expected) <= 1);
        CHECK(maxPerFrame <= Animator::kMaxStepsPerFrame);
        CHECK(c.steps >= 1);
    }
}

static void testClockDriven(Canvas* canvas) {
    printf("clock-driven animation\n");
    Animator animator(canvas, 40);
    Counter c(canvas, 0);
    animator.add(&c);
    animator.init();
    for (int i = 0; i < 200; i++) {
        animator.step();
    }
    CHECK(c.steps == 200);
    // Frame times add up to the elapsed time, and every frame is 25 ms apart.
    CHECK(c.totalFrameUs == c.elapsedUs());
    CHECK(c.elapsedUs() == 199 * 25000ull);  // its clock starts at its first frame
    for (size_t i = 0; i < c.times.size(); i++) {
        CHECK(c.times[i] % 25000 == 0);
        if (i > 0) {
            CHECK(c.times[i] - c.times[i - 1] == 25000);
        }
    }
}

static void testSharedClock(Canvas* canvas) {
    printf("frames on a shared clock, including a backwards jump\n");
    OffsetAnimator animator(canvas, 40);
    animator.offset = 7777777;  // show time is 7.78 s ahead of this Pico's clock
    Counter c(canvas, 0);
    animator.add(&c);
    animator.init();
    for (int i = 0; i < 100; i++) {
        animator.step();
        // Each frame starts exactly on a boundary of show time.
        CHECK(animator.getFrameTime() % 25000 == 0);
        CHECK(animator.clockUs() % 25000 == 0);  // and we sleep until the next one
    }
    uint64_t lastNow = c.now();
    // The shared clock steps 10 ms backwards (a sync correction).
    animator.offset -= 10000;
    for (int i = 0; i < 100; i++) {
        animator.step();
        CHECK(animator.getFrameTime() % 25000 == 0);
        CHECK(c.now() >= lastNow);  // an animation's clock never goes backwards
        lastNow = c.now();
    }
    CHECK(c.steps == 200);
    CHECK(animator.getMissedFrames() == 0);
}

static void testSlowFrames(Canvas* canvas) {
    printf("slow frames\n");
    Animator animator(canvas, 40);
    Counter c(canvas, 0);
    c.stepCost = 30000;  // longer than a frame
    animator.add(&c);
    animator.init();
    for (int i = 0; i < 20; i++) {
        animator.step();
        CHECK(animator.getFrameTime() % 25000 == 0);
    }
    CHECK(animator.getMissedFrames() == 20);
    // Frames still start on boundaries, two frame slots apart.
    for (size_t i = 1; i < c.times.size(); i++) {
        CHECK(c.times[i] - c.times[i - 1] == 25000 || c.times[i] - c.times[i - 1] == 50000);
    }
}

static void testTimedAnimation(Canvas* canvas) {
    printf("timed animations pass the clock on\n");
    Animator animator(canvas, 40);
    Counter a(canvas, 0), b(canvas, 0);
    animator.addTimed(&a, 1000);
    animator.add(&b);
    animator.init();
    for (int i = 0; i < 60; i++) {
        animator.step();
    }
    // About a second of a (it ends on the first step past 1 s), then b.
    printf("  a: %d steps, b: %d steps\n", a.steps, b.steps);
    CHECK(a.steps >= 40 && a.steps <= 42);
    CHECK(a.totalFrameUs == a.elapsedUs());
    CHECK(b.steps > 0);
    CHECK(b.times.front() % 25000 == 0);
    CHECK(b.totalFrameUs == b.elapsedUs());
}

/// @brief Starlight with its insides showing.
class OpenStarlight : public Starlight {
   public:
    using Starlight::Starlight;
    uint16_t getSwell() const { return swell; }
    int glinting() const {
        int n = 0;
        for (uint8_t g : glints) {
            n += g != 0;
        }
        return n;
    }
};

static void testStarlightAnyRate(Canvas* canvas) {
    printf("Starlight moves at the same speed at any frame rate\n");
    for (int fps : {20, 25, 40, 50}) {
        Animator animator(canvas, fps);
        OpenStarlight s(canvas, RGB(213, 150, 20), RGB(255, 240, 200), 3);
        animator.add(&s);
        animator.init();
        for (int i = 0; i < fps * 10; i++) {
            animator.step();
        }
        // The first frame has no time behind it, so 10 s of frames cover 10 s
        // minus one frame.
        uint32_t want = (uint32_t)(8100ull * (10000000 - 1000000 / fps) / 1000000) & 0xffff;
        printf("  %2d fps: swell %u (want %u), %d glinting\n", fps, s.getSwell(), want,
               s.glinting());
        CHECK(abs((int)s.getSwell() - (int)want) <= 1);
    }
}

int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);
    Canvas* canvas = new Canvas(10);
    Strip* strip = new Strip(0, 10);
    canvas->add(strip);
    canvas->setup();

    testLegacyRates(canvas);
    testClockDriven(canvas);
    testSharedClock(canvas);
    testSlowFrames(canvas);
    testTimedAnimation(canvas);
    testStarlightAnyRate(canvas);

    if (failures) {
        printf("%d check(s) FAILED\n", failures);
        return 1;
    }
    printf("all checks passed\n");
    return 0;
}
