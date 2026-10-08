#ifndef TWINKLEFOX_H
#define TWINKLEFOX_H

#pragma once

#include <vector>

#include "Animation.h"
#include "colorutils.h"

/// @brief Twinkling holiday lights that fade in and out, ported from Mark
/// Kriegsman's TwinkleFOX example for FastLED (MIT license).
///
/// Every pixel follows the same fade-up, fade-down wave, but each one runs on
/// its own clock: a random speed between 1x and nearly 3x, and a random
/// offset. The clock settings come from a little pseudo-random generator
/// that's reset to the same seed on every step, so they're the same each
/// time without storing anything per pixel. Each time a pixel's wave starts
/// over, it picks whether to light up at all this time around and which
/// palette color to use.
///
/// The animation cycles through a list of palettes, blending smoothly from
/// one to the next. If you don't add any, it uses the holiday palettes from
/// HolidayPalettes.h.
class TwinkleFox : public Animation {
   protected:
    std::vector<RGBPalette16> palettes;

    RGBPalette16 currentPalette;
    RGBPalette16 targetPalette;

    /// @brief Which palette in palettes we're showing or blending toward.
    int which = 0;

    /// @brief When we started showing the current palette, in ms.
    uint32_t paletteStart = 0;

    uint32_t msPerPalette;

    /// @brief 0 (very slow) to 8 (very fast).
    uint8_t speed;

    /// @brief 0 (none lit) to 8 (all lit at once).
    uint8_t density;

    RGB background = RGB::Black;

    bool coolLikeIncandescent = true;

    /// @brief Works out the color of one pixel at the given time on its own
    /// clock.
    RGB computeOneTwinkle(uint32_t ms, uint8_t salt);

   public:
    /// @brief Make a twinkle animation for a whole canvas.
    /// @param canvas the canvas to twinkle
    /// @param speed how fast the twinkles fade, from 0 (very slow) to 8 (very
    /// fast). 4 to 6 look good.
    /// @param density how many pixels are lit at once, from 0 (none) to 8 (all).
    /// @param secondsPerPalette how long to show each palette before blending
    /// to the next one.
    TwinkleFox(Canvas* canvas, uint8_t speed = 4, uint8_t density = 5,
               uint secondsPerPalette = 30);

    /// @brief Adds a palette to the list we cycle through.
    void addPalette(const RGBPalette16& palette);

    /// @brief Adds a palette made from a color map's colors to the list we
    /// cycle through. Since twinkles pick palette colors without blending,
    /// each twinkle will be exactly one of the map's colors.
    void addPalette(ColorMap* colorMap);

    /// @brief Sets the color for unlit pixels. Black by default; a very dim
    /// warm white looks like a string of lights that's on but resting.
    void setBackground(const RGB& color) { background = color; };

    /// @brief Whether twinkles redden a little as they fade, the way
    /// incandescent bulbs do. On by default.
    void setCoolLikeIncandescent(bool cool) { coolLikeIncandescent = cool; };

    void init() override;

    bool step() override;
};

#endif
