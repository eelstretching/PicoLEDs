#ifndef FILLLINE_H
#define FILLLINE_H
#pragma once

#include <stdint.h>

#include "Animation.h"
#include "Direction.h"
#include "stdlib.h"

class FillLine : public Animation {

    /// @brief The color we're drawing.
    uint8_t colorIndex;

    /// @brief Which direction we're moving.
    Direction direction = UP;

    /// @brief The size of the step that we take during the animation.
    uint8_t stepSize = 1;

    /// @brief The previous position that we drew at, or -1 if we haven't drawn.
    int16_t prevPos = -1;

    /// @brief The row or column where we're currently drawing.
    int16_t currPos;

    /// @brief Where we're heading to. We'll animate until we hit this point.
    int16_t endPos;

   public:
    FillLine(Canvas *canvas, ColorMap *colorMap, uint8_t colorIndex, Direction direction, int16_t startPos,
             int16_t endPos, uint8_t stepSize);

    ~FillLine() {};

    bool step() override;

   protected:
    /// @brief Draws the line as a row or columnat the given position.
    /// @param pos the position to draw at.
    void draw(uint16_t pos, uint8_t colorIndex);
};

#endif