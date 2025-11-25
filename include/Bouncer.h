#ifndef BOUNCER_H
#define BOUNCER_H

#pragma once

#include "Animation.h"
#include "Canvas.h"

class Bouncer : public Animation {
    int x;

    int maxX;

    int y;

    int maxY;

    bool up;

    bool right;

    uint8_t color;

   public:
    /// @brief Creates a bouncer at the given coordinates
    /// @param canvas The canvas we'll draw on.
    /// @param x the initial x position of the bouncer
    /// @param y the initial y position of the bouncer
    Bouncer(Canvas* canvas, ColorMap* colorMap, uint8_t color, int x, int y);

    Bouncer(Canvas* canvas, ColorMap* colorMap, uint8_t color, int x, int y,
            int maxX, int maxY);
    int getFPSNeeded() { return 10; };
    bool step();
};

#endif