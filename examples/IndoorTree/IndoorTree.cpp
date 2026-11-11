//
// Traditional Christmas lights for a real tree indoors: four or five strands
// of 200 pixels wound on the tree, run by a Pico 2 W through an output
// (bank) board.
//
// The show goes round, slowly, through warm white and old-fashioned
// multicolor (red, green, blue, orange, yellow) in the patterns an old light
// controller had: steady, twinkle, slo-glo, waves, a slow chase, plus the
// gold Starlight and some TwinkleFox. Every pattern fades in from the one
// before over a few seconds, so nothing ever cuts. One time round is about
// half an hour, and steady warm white and steady multicolor get the longest
// turns.
//
// Wiring is the same as BankTest (Pico 2 W GPIO -> ribbon pin -> output):
//
//   GP2 (pin 4)  -> ribbon 2  -> output 1
//   GP3 (pin 5)  -> ribbon 4  -> output 2
//   GP4 (pin 6)  -> ribbon 6  -> output 3
//   GP5 (pin 7)  -> ribbon 8  -> output 4
//   GP6 (pin 9)  -> ribbon 10 -> output 5
//
// Ribbon odd pins 1-11 to Pico ground, 13 to 3V3 (pin 36), 15 to VBUS
// (pin 40). Strands that aren't plugged in don't matter, so NUM_STRANDS 5
// works with four strands too.
//
// Progress is printed on USB serial (and UART0), and the Pico's LED blinks
// once a second as a heartbeat.
#include <vector>

#include "Animator.h"
#include "Canvas.h"
#include "ClassicLights.h"
#include "HolidayPalettes.h"
#include "Starlight.h"
#include "Strip.h"
#include "TwinkleFox.h"
#include "pico/cyw43_arch.h"
#include "pico/printf.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"

#define NUM_STRANDS 5
#define START_PIN 2
#define STRAND_LEN 200
//
// Out of 255. Seed pixels are bright, and this is a living room: 40 is a
// cozy glow, and keeps five strands to tens of watts even at full white.
#define BRIGHTNESS 40
//
// How long each pattern fades in from the one before.
#define FADE_MS 4000

//
// A minute, in ms. Build the simulator with -DMINUTE_MS=3000 to see the whole
// show go round in a couple of minutes.
#ifndef MINUTE_MS
#define MINUTE_MS 60000
#endif
#define MINUTES(m) ((m) * MINUTE_MS)

class IndoorAnimator : public Animator {
   public:
    IndoorAnimator(Canvas* canvas) : Animator(canvas) {}
    void animationChanged() override {
        printf("Now showing %s\n", animations[pos]->getName());
    }
};

static Animator* show;
static std::vector<Animation*> keep;

//
// Adds an animation to the show, fading in from what came before.
static void add(Animation* a, const char* name, int ms) {
    a->setName(name);
    CrossFade* f = new CrossFade(a, FADE_MS);
    keep.push_back(a);
    keep.push_back(f);
    show->addTimed(f, ms);
}

int main() {
    stdio_init_all();
    //
    // The cyw43 driver claims its PIO state machine first, so the renderer
    // can't take it.
    bool led = cyw43_arch_init() == 0;
    if (!led) {
        printf("cyw43 init failed, no heartbeat LED\n");
    }

    Canvas canvas(STRAND_LEN);
    for (int i = 0; i < NUM_STRANDS; i++) {
        Strip* s = new Strip(START_PIN + i, STRAND_LEN);
        s->setColorOrder(ColorOrder::ORGB);
        canvas.add(s);
    }
    canvas.setBrightness(BRIGHTNESS);
    //
    // Smooth slow fades at low brightness.
    canvas.setDithering(true);
    canvas.setup();
    canvas.clear();
    canvas.show();

    IndoorAnimator animator(&canvas);
    show = &animator;

    std::vector<RGB> warm = {ClassicLights::WarmWhite};
    const std::vector<RGB>& multi = ClassicLights::Multicolor;
    std::vector<RGB> redGreen = {RGB(255, 18, 0), RGB(0, 170, 25)};

    add(new ClassicLights(&canvas, warm), "Warm white", MINUTES(5));

    ClassicLights* warmTwinkle =
        new ClassicLights(&canvas, warm, ClassicMode::TWINKLE);
    add(warmTwinkle, "Warm twinkle", MINUTES(3));

    add(new ClassicLights(&canvas, multi), "Multicolor", MINUTES(5));

    add(new ClassicLights(&canvas, multi, ClassicMode::TWINKLE),
        "Multi twinkle", MINUTES(3));

    add(new ClassicLights(&canvas, multi, ClassicMode::SLO_GLO), "Slo-glo",
        MINUTES(2));

    add(new Starlight(&canvas, RGB(213, 150, 20), RGB(255, 240, 200), 15),
        "Starlight", MINUTES(3));

    add(new ClassicLights(&canvas, multi, ClassicMode::IN_WAVES), "In waves",
        MINUTES(2));

    //
    // Old C9s twinkling, slow, with the string resting at a dim warm glow
    // rather than going dark.
    TwinkleFox* c9 = new TwinkleFox(&canvas, 2, 6, 600);
    c9->addPalette(RetroC9Palette);
    c9->setBackground(RGB(12, 5, 1));
    add(c9, "C9 twinkle", MINUTES(3));

    add(new ClassicLights(&canvas, redGreen, ClassicMode::SLOW_CHASE),
        "Red-green chase", MINUTES(2));

    TwinkleFox* fairy = new TwinkleFox(&canvas, 2, 7, 600);
    fairy->addPalette(FairyLightPalette);
    add(fairy, "Fairy lights", MINUTES(3));

    printf("IndoorTree: %d strands on GP%d-GP%d, %d pixels each, brightness %d\n",
           NUM_STRANDS, START_PIN, START_PIN + NUM_STRANDS - 1, STRAND_LEN,
           BRIGHTNESS);

    animator.init();
    bool ledOn = false;
    uint64_t nextBlink = 0;
    while (true) {
        animator.step();
        if (led && time_us_64() >= nextBlink) {
            ledOn = !ledOn;
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, ledOn);
            nextBlink = time_us_64() + 500000;
        }
        if (animator.getFrameCount() % 2400 == 0) {
            animator.printStats();
        }
    }
}
