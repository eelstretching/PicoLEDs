#include "LinesFill.h"

#include <stdint.h>

LinesFill::LinesFill(Canvas* canvas, int nBands, Direction direction,
                   int stepSize)
    : canvas(canvas),
      nBands(nBands),
      direction(direction),
      stepSize(stepSize) {
    //
    // How big are the bands of colors? We're doing integer division, so we
    // might miss a few rows at the bottom. We can make things more complicated
    // on the last one later if we need to.
    switch (direction) {
        case UP:
        case DOWN:
            bandSize = canvas->getWidth() / nBands;
            nLines = canvas->getWidth();
            break;
        case LEFT:
        case RIGHT:
            bandSize = canvas->getHeight() / nBands;
            nLines = canvas->getHeight();
            break;
    }
    lines = new FillLine*[nLines];
    for (int i = 0; i < nLines; i++) {
        lines[i] = nullptr;
    }
}

void LinesFill::init() {
    canvas->clear();
    uint16_t startPos;
    uint16_t endPos;
    switch (direction) {
        case UP:
            startPos = 0;
            endPos = (canvas->getWidth() - 1);
            break;
        case DOWN:
            startPos = canvas->getWidth() - 1;
            endPos = 0;
            break;
        case LEFT:
            startPos = 0;
            endPos = (canvas->getHeight() - 1);
            break;
        case RIGHT:
            startPos = canvas->getHeight() - 1;
            endPos = 0;
            break;
    }

    uint8_t currBand = 0;
    for (int i = 0; i < nLines; i++) {
        //
        // Get rid of the old line, if there is one. There shouldn't be, but
        // just to be safe.
        if (lines[i] != nullptr) {
            delete lines[i];
        }

        //
        // New line. We'll start with most of them off the canvas.
        lines[i] = new FillLine(canvas, currBand, direction, startPos,
                                endPos, stepSize);

        //
        // Positions for the next line, we'll leave a gap between lines while
        // they're being animated.
        switch (direction) {
            case UP:
            case LEFT:
                startPos -= (gap + 1);
                endPos--;
                break;
            case DOWN:
            case RIGHT:
                startPos += (gap + 1);
                endPos++;
                break;
        }
        if ((i + 1) % bandSize == 0) {
            currBand++;
        }
    }
}

bool LinesFill::step() {
    //
    // How many lines are finished?
    int nf = 0;
    for (int i = 0; i < nLines; i++) {
        if (lines[i] != nullptr) {
            //
            // Step the line, and if it finishes, take it out of the running.
            if (!lines[i]->step()) {
                nf++;
                delete lines[i];
                lines[i] = nullptr;
            }
        } else {
            nf++;
        }
    }
    //
    // If we finished all the lines, then we're done.
    return nf != nLines;
}
