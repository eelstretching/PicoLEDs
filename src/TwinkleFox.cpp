#include "TwinkleFox.h"

#include "HolidayPalettes.h"
#include "math8.h"

/// @brief Like a triangle wave, but with a faster attack and a slower decay.
static uint8_t attackDecayWave8(uint8_t i) {
    if (i < 86) {
        return i * 3;
    }
    i -= 86;
    return 255 - (i + (i / 2));
}

/// @brief If we're in the fading-down half of the wave, take a little green
/// and more blue out, the way an incandescent bulb reddens as it dims.
static void coolLikeIncandescentBulb(RGB& c, uint8_t phase) {
    if (phase < 128) {
        return;
    }
    uint8_t cooling = (phase - 128) >> 4;
    c.g = qsub8(c.g, cooling);
    c.b = qsub8(c.b, cooling * 2);
}

TwinkleFox::TwinkleFox(Canvas* canvas, uint8_t speed, uint8_t density,
                       uint secondsPerPalette)
    : Animation(canvas, nullptr),
      msPerPalette(secondsPerPalette * 1000),
      speed(MIN(speed, 8)),
      density(MIN(density, 8)) {}

void TwinkleFox::addPalette(const RGBPalette16& palette) {
    palettes.push_back(palette);
}

void TwinkleFox::addPalette(ColorMap* colorMap) {
    RGBPalette16 p;
    colorMap->toPalette(p);
    palettes.push_back(p);
}

void TwinkleFox::init() {
    if (palettes.empty()) {
        palettes = {RetroC9Palette,   BlueWhitePalette, FairyLightPalette,
                    RedGreenWhitePalette, RedWhitePalette, SnowPalette,
                    HollyPalette,     IcePalette};
    }
    which = 0;
    currentPalette = palettes[0];
    targetPalette = palettes[0];
    paletteStart = to_ms_since_boot(get_absolute_time());
}

RGB TwinkleFox::computeOneTwinkle(uint32_t ms, uint8_t salt) {
    //
    // The low bits of the time drive the brightness wave. The high bits pick
    // the color and whether we light up at all, so they stay the same for the
    // whole of one fade up and down.
    uint16_t ticks = ms >> (8 - speed);
    uint8_t fastcycle8 = ticks;
    uint16_t slowcycle16 = (ticks >> 8) + salt;
    slowcycle16 += sin8(slowcycle16);
    slowcycle16 = (slowcycle16 * 2053) + 1384;
    uint8_t slowcycle8 = (slowcycle16 & 0xFF) + (slowcycle16 >> 8);

    uint8_t bright = 0;
    if (((slowcycle8 & 0x0E) / 2) < density) {
        bright = attackDecayWave8(fastcycle8);
    }

    if (bright == 0) {
        return RGB::Black;
    }
    uint8_t hue = slowcycle8 - salt;
    RGB c = ColorFromPalette(currentPalette, hue, bright, NOBLEND);
    if (coolLikeIncandescent) {
        coolLikeIncandescentBulb(c, fastcycle8);
    }
    return c;
}

bool TwinkleFox::step() {
    aw.start();
    uint32_t now = to_ms_since_boot(get_absolute_time());

    //
    // Time for the next palette?
    if (palettes.size() > 1 && now - paletteStart >= msPerPalette) {
        which = (which + 1) % palettes.size();
        targetPalette = palettes[which];
        paletteStart = now;
    }
    nblendPaletteTowardPalette(currentPalette, targetPalette, 48);

    uint8_t backgroundBrightness = background.getAverageLight();

    //
    // This must start from the same seed every step, so that every pixel gets
    // the same clock speed and offset every time.
    uint16_t prng16 = 11337;
    for (int y = 0; y < canvas->getHeight(); y++) {
        for (int x = 0; x < canvas->getWidth(); x++) {
            prng16 = (uint16_t)(prng16 * 2053) + 1384;
            uint16_t myclockoffset16 = prng16;
            prng16 = (uint16_t)(prng16 * 2053) + 1384;
            //
            // A clock speed in eighths, from 8/8 to 23/8.
            uint8_t myspeedmultiplierQ5_3 =
                ((((prng16 & 0xFF) >> 4) + (prng16 & 0x0F)) & 0x0F) + 0x08;
            uint32_t myclock30 =
                (uint32_t)((now * myspeedmultiplierQ5_3) >> 3) +
                myclockoffset16;
            uint8_t myunique8 = prng16 >> 8;

            RGB c = computeOneTwinkle(myclock30, myunique8);

            //
            // Use the twinkle if it's clearly brighter than the background,
            // blend the two if it's only a bit brighter, otherwise show the
            // background.
            int16_t deltabright = c.getAverageLight() - backgroundBrightness;
            if (deltabright >= 32 || !background) {
                canvas->set(x, y, c);
            } else if (deltabright > 0) {
                canvas->set(x, y, blend(background, c, deltabright * 8));
            } else {
                canvas->set(x, y, background);
            }
        }
    }
    aw.finish();
    return true;
}
