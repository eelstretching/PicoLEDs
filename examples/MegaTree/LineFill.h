#ifndef LINEFILL_H
#define LINEFILL_H

#pragma once

#include <stdint.h>
#include "Animation.h"  
#include "Direction.h"

enum State {
    FILLING,
    FLASHING
};

class LineFill : public Animation {

    Canvas *canvas;

    /// @brief The colors to use for the fill. The first n of these colors will be used, so it should be at least as big as that.
    RGB *colors;

    /// @brief The number of color stripes to use. 
    int n;

    /// @brief The direction that we're filling to.
    Direction direction;

    /// @brief The current state of the animation, either filling or flashing a completed line.
    State state;

    /// @brief The points in the canvas where we should change colors.
    int *divisions;

    /// @brief The current position that we're drawing.
    int currPos;

    /// @brief The current target that we're filling towards.
    int currTarget;

    /// @brief The index into the colors and targets that we're currently using.
    int currIndex = 0;

    int flashCount = 5;

    int currFlash = 0;

   public:
    LineFill(Canvas *canvas, RGB *colors, int n, Direction direction = DOWN);

    ~LineFill() {};

    int getFPSNeeded() override { return 120; };

    bool step() override;
};

#endif