#include "Starlight.h"

#include "colorutils.h"
#include "math8.h"

/// @brief How far a glint moves through its 256-step rise and fall each
/// second: about 0.8 s to rise and 1.6 s to fade.
#define GLINT_SPEED 90

/// @brief How fast the swells move, in 1/65536ths of a cycle per second: two
/// swells pass about every eight seconds.
#define SWELL_SPEED 8100

/// @brief How much of something moving at ratePerSecond is due in frameUs
/// microseconds, carrying the fraction over to the next frame so nothing is
/// lost to rounding at any frame rate.
static uint32_t due(uint32_t ratePerSecond, uint32_t frameUs, uint32_t& carry) {
    uint64_t total = (uint64_t)ratePerSecond * frameUs + carry;
    carry = (uint32_t)(total % 1000000);
    return (uint32_t)(total / 1000000);
}

/// @brief Eases in and out, so a glint starts and ends gently: slow at 0 and
/// 255, fastest in the middle.
static uint8_t easeInOut(uint8_t i) {
    uint8_t j = i < 128 ? i : 255 - i;
    uint8_t e = scale8(j, j) * 2;
    return i < 128 ? e : 255 - e;
}

/// @brief A glint's brightness partway through: a gentle rise over the first
/// third and a slower fall over the rest.
static uint8_t glintBrightness(uint8_t phase) {
    if (phase < 86) {
        return easeInOut(phase * 3);
    }
    uint8_t fall = (phase - 86) + ((phase - 86) >> 1);
    return easeInOut(255 - fall);
}

Starlight::Starlight(Canvas* canvas, const RGB& glow, const RGB& glint,
                     uint glintsPerSecond)
    : Animation(canvas, nullptr, 0),
      glow(glow),
      glint(glint),
      glintsPerSecond(glintsPerSecond) {}

void Starlight::init() {
    glints.assign(canvas->getWidth() * canvas->getHeight(), 0);
    owed = 0;
    swell = 0;
    glintCarry = 0;
    phaseCarry = 0;
    swellCarry = 0;
}

bool Starlight::step() {
    aw.start();
    int w = canvas->getWidth();
    int h = canvas->getHeight();
    if (glints.size() != (size_t)(w * h)) {
        glints.assign(w * h, 0);
    }

    //
    // Start the glints that are due this frame on pixels that aren't already
    // glinting.
    // Starlight goes by the clock (its fps is 0), so it looks the same at any
    // frame rate: everything moves by how much time has passed.
    owed += due(glintsPerSecond, getFrameUs(), glintCarry);
    while (owed > 0) {
        owed--;
        uint p = random16(glints.size());
        if (glints[p] == 0) {
            glints[p] = 1;
        }
    }
    uint32_t glintStep = due(GLINT_SPEED, getFrameUs(), phaseCarry);

    //
    // The swells go up the canvas (along x, which is up the strands on a
    // MegaTree), two at a time, taking about eight seconds to pass.
    swell += due(SWELL_SPEED, getFrameUs(), swellCarry);
    uint8_t swellPhase = swell >> 8;

    //
    // Each pixel's own brightness comes from a little pseudo-random generator
    // that starts from the same seed every step, so it stays the same without
    // storing it.
    uint16_t prng16 = 11337;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            prng16 = (uint16_t)(prng16 * 2053) + 1384;
            uint8_t own = 176 + ((prng16 >> 8) % 80);

            //
            // Between 60% and 100% brightness as the swells pass.
            uint8_t s = sin8(swellPhase - (uint8_t)(x * 512 / w));
            uint8_t level = scale8(own, 153 + scale8(s, 102));
            RGB c = glow.scale8(level);

            uint8_t& g = glints[y * w + x];
            if (g != 0) {
                c = blend(c, glint, glintBrightness(g));
                g = g + glintStep > 255 ? 0 : g + glintStep;
            }
            canvas->set(x, y, c);
        }
    }
    aw.finish();
    return true;
}
