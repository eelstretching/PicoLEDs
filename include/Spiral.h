#ifndef SPIRAL_H
#define SPIRAL_H

#pragma once

#include <stdint.h>

#include "Animation.h"

/// @brief An animation that draws a spiral pattern on the tree.
class Spiral : public Animation {
    uint8_t width;
    uint8_t length;
    uint8_t barWidth;
    uint8_t nColors;
    uint8_t *colors;

    int x = 0;
    int y = 0;

   public:
    /// @brief Construct a Spiral animation.
    /// @param canvas The canvas to draw on.
    /// @param width The width of the spiral in pixels.
    /// @param length The length of the spiral in pixels.
    Spiral(Canvas* canvas, ColorMap* colorMap, uint8_t nColors, uint8_t *colors, uint8_t width, uint8_t length);

    /// @brief Default destructor.
    ~Spiral();

    void init() override;

    /// @brief Gets the frames-per-second this animation requires. Default
    /// is 30.
    /// @return the FPS this animation needs.
    int getFPSNeeded() override { return 50; };

    /// @brief Takes one step in the animation.
    /// @returns true if the animation will continue after this step, false if
    /// the animation is complete.
    bool step() override;
};

#endif
