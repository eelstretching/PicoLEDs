#ifndef LINEFILL_H
#define LINEFILL_H

#pragma once

#include "Animation.h"
#include "Direction.h"
#include "FillLine.h"

enum State { FILLING, FLASHING };

class LineFill : public Animation {
    Canvas *canvas;

    /// @brief The number of color bands to use.
    int nBands;

    /// @brief The number of lines that we'll be animating.
    int nLines;

    /// @brief The direction that we're filling to.
    Direction direction;

    /// @brief How big a step to take during animation.
    int stepSize;

    /// @brief How big a band of colors is.
    int bandSize;

    /// @brief How big a gap to leave between lines as they move up.
    int gap = 1;

    FillLine **lines;

   public:
    LineFill(Canvas *canvas, int nBands, Direction direction = UP,
             int stepSize = 1);

    ~LineFill() {};

    int getFPSNeeded() override { return 80; };

    bool step() override;

    void init() override;

    void setGap(int gap) { this->gap = gap; };

};

#endif