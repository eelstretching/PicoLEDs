#include "Starlight.h"

#include "colorutils.h"
#include "math8.h"

/// @brief How far a glint moves through its rise and fall each frame. At 30
/// fps it takes about 0.8 s to rise and 1.6 s to fade.
#define GLINT_STEP 3

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
    : Animation(canvas, nullptr),
      glow(glow),
      glint(glint),
      glintsPerSecond(glintsPerSecond) {}

void Starlight::init() {
    glints.assign(canvas->getWidth() * canvas->getHeight(), 0);
    owed = 0;
    swell = 0;
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
    owed += glintsPerSecond * 256 / MAX(getFPS(), 1);
    while (owed >= 256) {
        owed -= 256;
        uint p = random16(glints.size());
        if (glints[p] == 0) {
            glints[p] = 1;
        }
    }

    //
    // The swells go up the canvas (along x, which is up the strands on a
    // MegaTree), two at a time, taking about eight seconds to pass.
    swell += 270;
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
                g = g > 255 - GLINT_STEP ? 0 : g + GLINT_STEP;
            }
            canvas->set(x, y, c);
        }
    }
    aw.finish();
    return true;
}
