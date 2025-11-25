#include "FillLine.h"

#include <stdint.h>

FillLine::FillLine(Canvas* canvas, ColorMap *colorMap, uint8_t colorIndex, Direction direction, int16_t startPos,
                   int16_t endPos, uint8_t stepSize)
    : Animation(canvas, colorMap),
      colorIndex(colorIndex),
      direction(direction),
      currPos(startPos),
      endPos(endPos),
      stepSize(stepSize) {}

bool FillLine::step() {
    // Clear the previous line, if it's on the canvas.
    if (prevPos >= 0) {
        draw(prevPos, canvas->getBackgroundIndex());
    }
    //
    // Draw the current line.
    draw(currPos, colorIndex);

    //
    // If the current position is the end position, then we're done.
    if (currPos == endPos) {
        return false;
    }

    prevPos = currPos;
    //
    // Take a step in the right direction, being careful not to overshoot the
    // ending position.
    switch (direction) {
        case UP:
        case LEFT:
            currPos += stepSize;
            if (currPos > endPos) {
                currPos = endPos;
            }
            break;

        case DOWN:
        case RIGHT:
            currPos -= stepSize;
            if (currPos < endPos) {
                currPos = endPos;
            }
            break;
    }

    return true;
}

void FillLine::draw(uint16_t pos, uint8_t colorIndex) {
    switch (direction) {
        case UP:
        case DOWN:
            canvas->fillColumn(pos, colorIndex);
            break;
        case LEFT:
        case RIGHT:
            canvas->fillRow(pos, colorIndex);
            break;
    }
}
