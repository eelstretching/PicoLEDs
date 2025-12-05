#include "RotRandRows.h"

RotRandRows::RotRandRows(Canvas* canvas, ColorMap* colorMap, uint8_t nColors)
    : Animation(canvas, colorMap), nColors(nColors) {}

void RotRandRows::init() {
    firstFrame = true;
    canvas->setColorMap(colorMap);
    for (int i = 0; i < nColors; i++) {
        bool unique = false;
        uint8_t v;
        //
        // Make sure we don't repeat colors.
    uniqueLoop:
        while (!unique) {
            v = random8(colorMap->getUsed());
            for (int j = 0; j < i; j++) {
                if (currColors[j] == v) {
                    goto uniqueLoop;
                }
            }
            unique = true;
        }
        currColors[i] = v;
    }

    //
    // Random column width, but not too small or big.
    rowWidth = random8(canvas->getHeight()/3);
    rowWidth += 3;
    uint8_t nRows = (canvas->getHeight() / rowWidth);
    if (canvas->getHeight() % rowWidth == 0) {
        nRows++;
    }

    if (nRows % 2 == 1) {
        if (nColors % 2 == 0) {
            rowWidth += 1;
        }
    } else {
        if (nColors % 2 == 1) {
            rowWidth += 1;
        }
    }
}

bool RotRandRows::step() {
    if (firstFrame) {
        for (int i = 0; i < canvas->getHeight(); i++) {
            uint8_t colorIndex = (i / rowWidth) % nColors;
            canvas->fillRow(i, colorIndex);
        }
        firstFrame = false;
    } else {
        canvas->rotateUp();
    }
    return true;
}
