#include "ClassicLights.h"

#include "colorutils.h"
#include "math8.h"

const RGB ClassicLights::WarmWhite(255, 136, 36);

const std::vector<RGB> ClassicLights::Multicolor = {
    RGB(255, 18, 0),   // red
    RGB(0, 170, 25),   // green
    RGB(10, 30, 230),  // blue
    RGB(255, 80, 0),   // orange
    RGB(255, 160, 20), // yellow
};

/// @brief How far a twinkle gets through its 256 steps each second: about
/// 1.3 s to dim out, rest dark, and glow back.
#define TWINKLE_SPEED 200

static uint32_t due(uint32_t ratePerSecond, uint32_t frameUs, uint32_t& carry) {
    uint64_t total = (uint64_t)ratePerSecond * frameUs + carry;
    carry = (uint32_t)(total % 1000000);
    return (uint32_t)(total / 1000000);
}

/// @brief Slow at 0 and 255, fastest in the middle.
static uint8_t easeInOut(uint8_t i) {
    uint8_t j = i < 128 ? i : 255 - i;
    uint8_t e = scale8(j, j) * 2;
    return i < 128 ? e : 255 - e;
}

/// @brief A twinkling bulb's brightness partway through: a fade out over the
/// first 30%, dark for a moment, and a slower glow back up, the way a
/// filament cools quickly and warms up again.
static uint8_t twinkleLevel(uint8_t phase) {
    if (phase < 80) {
        return 255 - easeInOut(phase * 3 + phase / 5);
    }
    if (phase < 120) {
        return 0;
    }
    return easeInOut((phase - 120) * 255 / 135);
}

RGB ClassicLights::dimBulb(const RGB& c, uint8_t level) {
    uint8_t g = scale8(level, 160 + scale8(level, 95));
    uint8_t b = scale8(level, level);
    return RGB(scale8(c.r, level), scale8(c.g, g), scale8(c.b, b));
}

ClassicLights::ClassicLights(Canvas* canvas, const std::vector<RGB>& colors,
                             ClassicMode mode, uint spacing)
    : Animation(canvas, nullptr, 0),
      colors(colors),
      mode(mode),
      spacing(spacing < 1 ? 1 : spacing) {
    if (this->colors.empty()) {
        this->colors.push_back(WarmWhite);
    }
    switch (mode) {
        case ClassicMode::SLO_GLO:
            periodMs = 8000;
            floor = 20;
            break;
        case ClassicMode::IN_WAVES:
            periodMs = 6000;
            floor = 50;
            break;
        case ClassicMode::SLOW_FADE:
            periodMs = 12000;
            floor = 70;
            break;
        case ClassicMode::SLOW_CHASE:
            periodMs = 4000;
            floor = 40;
            break;
        default:
            periodMs = 10000;
            floor = 0;
            break;
    }
}

void ClassicLights::init() {
    twinkles.assign(canvas->getWidth() * canvas->getHeight(), 0);
    owed = 0;
    twinkleCarry = 0;
    phaseCarry = 0;
}

uint8_t ClassicLights::cycle() {
    uint64_t periodUs = (uint64_t)periodMs * 1000;
    if (periodUs == 0) {
        return 0;
    }
    return (uint8_t)((elapsedUs() % periodUs) * 256 / periodUs);
}

uint8_t ClassicLights::level(uint8_t c, int n, int x, int y) {
    uint8_t wave;
    switch (mode) {
        case ClassicMode::SLO_GLO:
            //
            // Two circuits, half a cycle apart. Start at the bright end so
            // the fade in doesn't look like a flash.
            wave = sin8(c + 64 + (n % 2) * 128);
            break;
        case ClassicMode::IN_WAVES:
            //
            // A wave every 60 pixels or so along the strand.
            wave = sin8(c - (uint8_t)(x * 256 / 60));
            break;
        case ClassicMode::SLOW_FADE:
            wave = sin8(c + 64);
            break;
        case ClassicMode::SLOW_CHASE:
            wave = sin8(c - (n % 4) * 64);
            break;
        default:
            return 255;
    }
    return floor + scale8(easeInOut(wave), 255 - floor);
}

bool ClassicLights::step() {
    aw.start();
    int w = canvas->getWidth();
    int h = canvas->getHeight();
    if (twinkles.size() != (size_t)(w * h)) {
        twinkles.assign(w * h, 0);
    }

    uint32_t twinkleStep = 0;
    if (mode == ClassicMode::TWINKLE) {
        uint bulbs = (w + spacing - 1) / spacing * h;
        //
        // owed counts hundredths of a twinkle, since the rate is per 100
        // bulbs.
        owed += due(twinklesPer100 * bulbs, getFrameUs(), twinkleCarry);
        while (owed >= 100) {
            owed -= 100;
            uint p = random16(twinkles.size());
            if (twinkles[p] == 0 && (p % w) % spacing == 0) {
                twinkles[p] = 1;
            }
        }
        twinkleStep = due(TWINKLE_SPEED, getFrameUs(), phaseCarry);
    }

    uint8_t c = cycle();

    //
    // Each bulb's own brightness, 78% to 100%, from a little generator that
    // starts from the same seed every step, so it stays put without storing
    // it.
    uint16_t prng16 = 4242;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            if (x % spacing != 0) {
                canvas->set(x, y, RGB::Black);
                continue;
            }
            prng16 = (uint16_t)(prng16 * 2053) + 1384;
            uint8_t own = 200 + ((prng16 >> 8) % 56);
            int n = x / spacing;
            uint8_t l = scale8(own, level(c, n, x, y));
            uint8_t& t = twinkles[y * w + x];
            if (t != 0) {
                l = scale8(l, twinkleLevel(t));
                t = t + twinkleStep > 255 ? 0 : t + twinkleStep;
            }
            canvas->set(x, y, dimBulb(colors[(n + y) % colors.size()], l));
        }
    }
    aw.finish();
    return true;
}

CrossFade::CrossFade(Animation* animation, uint32_t fadeMs)
    : Animation(animation->getCanvas(), nullptr, 0),
      animation(animation),
      fadeUs(fadeMs * 1000) {}

void CrossFade::init() {
    int w = canvas->getWidth();
    int h = canvas->getHeight();
    from.resize(w * h);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            from[y * w + x] = canvas->get(x, y);
        }
    }
    animation->init();
}

bool CrossFade::step() {
    bool more = animation->step();
    uint64_t e = elapsedUs();
    if (e >= fadeUs || from.empty()) {
        //
        // Done fading; let the memory go.
        if (!from.empty()) {
            std::vector<RGB>().swap(from);
        }
        return more;
    }
    uint8_t amount = easeInOut((uint8_t)(e * 255 / fadeUs));
    int w = canvas->getWidth();
    int h = canvas->getHeight();
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            canvas->set(x, y, blend(from[y * w + x], canvas->get(x, y), amount));
        }
    }
    return more;
}
