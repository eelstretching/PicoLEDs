#include "LineFill.h"

LineFill::LineFill(Canvas* canvas, RGB* colors, int nBands, Direction direction,
                   int stepSize)
    : canvas(canvas),
      colors(colors),
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
            break;
        case LEFT:
        case RIGHT:
            bandSize = canvas->getHeight() / nBands;
            break;
    }
    lines = new FillLine*[bandSize];
    for(int i = 0; i < bandSize; i++) {
        lines[i] = nullptr;
    }
}

void LineFill::newBand() {
    uint16_t startPos;
    uint16_t endPos;
    switch (direction) {
        case UP:
            startPos = 0;
            endPos = (canvas->getWidth() - 1) - (bandSize * currBand);
            break;
        case DOWN:
            startPos = canvas->getWidth() - 1;
            endPos = (bandSize * currBand);
            break;
        case LEFT:
            startPos = 0;
            endPos = (canvas->getHeight() - 1) - (bandSize * currBand);
            break;
        case RIGHT:
            startPos = canvas->getHeight() - 1;
            endPos = (bandSize * currBand);
            break;
    }
    printf("New band %d startPos=%d endPos=%d\n", currBand, startPos,
           endPos);
    for (int i = 0; i < bandSize; i++) {
        //
        // Get rid of the old line, if there is one. There shouldn't be, but just to be safe.
        if (lines[i] != nullptr) {
            delete lines[i];
        }

        //
        // New line. We'll start with most of them off the canvas.
        lines[i] = new FillLine(canvas, colors[currBand], direction, startPos,
                                endPos, stepSize);

        //
        // Positions for the next line, we'll leave a gap between lines while
        // they're being animated.
        switch (direction) {
            case UP:
            case LEFT:
                startPos -= (gap+1);
                endPos--;
                break;
            case DOWN:
            case RIGHT:
                startPos += (gap+1);
                endPos++;
                break;
        }
    }
}

void LineFill::init() {
    canvas->clear();
    currBand = 0;
    newBand();
}

bool LineFill::step() {
    int nf = 0;
    for (int i = 0; i < bandSize; i++) {
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
    // If we finished all the lines in this band, move on to the next band.
    if (nf == bandSize) {
        currBand++;
        if (currBand == nBands) {
            return false;
        }
        newBand();
    }
    return true;
}
