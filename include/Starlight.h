#ifndef STARLIGHT_H
#define STARLIGHT_H

#pragma once

#include <vector>

#include "Animation.h"

/// @brief A calm, warm glow for the whole canvas. Every pixel glows gold,
/// each a little brighter or dimmer than its neighbours like real bulbs, and
/// slow swells of brightness drift up the tree. Now and then a pixel slowly
/// brightens to a warm white glint and fades back down again.
///
/// It's meant to be slow: a glint takes a couple of seconds to come and go,
/// and a swell takes several seconds to pass. Christmas is a slower time.
class Starlight : public Animation {
   protected:
    /// @brief The color everything glows.
    RGB glow;

    /// @brief The color of the glints at their brightest.
    RGB glint;

    /// @brief How many glints start each second, across the whole canvas.
    uint glintsPerSecond;

    /// @brief How far each glint has got through its rise and fall, or 0
    /// for a pixel that isn't glinting.
    std::vector<uint8_t> glints;

    /// @brief Glints owed but not started yet, in 1/256ths of a glint.
    uint owed = 0;

    /// @brief Where the swells are, out of 65536 of a full cycle.
    uint16_t swell = 0;

   public:
    /// @param canvas the canvas to draw on
    /// @param glow the color everything glows
    /// @param glint the color of the glints at their brightest
    /// @param glintsPerSecond how many glints start each second across the
    /// whole canvas
    Starlight(Canvas* canvas, const RGB& glow = RGB(213, 150, 20),
              const RGB& glint = RGB(255, 240, 200),
              uint glintsPerSecond = 30);

    void init() override;
    bool step() override;
};

#endif
