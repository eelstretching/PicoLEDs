#ifndef LINEFILL_H
#define LINEFILL_H

#pragma once

#include <stdint.h>

#include "Animation.h"
#include "Direction.h"
#include "FillLine.h"

enum FillState { FILLING, FLASHING };

class LinesFill : public Animation {

    /// @brief The number of color bands to use.
    int nColors;

    uint8_t *colors;

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
    LinesFill(Canvas *canvas, ColorMap *colorMap, int nColors, uint8_t *colors, Direction direction = UP,
             int stepSize = 1);

    ~LinesFill() {};

    int getFPSNeeded() override { return 60; };

    bool step() override;

    void init() override;

    void setGap(int gap) { this->gap = gap; };

};

#endif