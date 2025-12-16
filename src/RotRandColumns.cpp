#include "RotRandColumns.h"

RotRandColumns::RotRandColumns(Canvas* canvas, ColorMap* colorMap, uint8_t nColors)
    : Animation(canvas, colorMap, 50), nColors(nColors) {}

void RotRandColumns::init() {
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
    colWidth = random16(5, canvas->getWidth() / 4);
    uint8_t nColumns = (canvas->getWidth() / colWidth);
    if(canvas->getWidth() % colWidth == 0) {
        nColumns++;
    }

    //
    // Try to keep the start and end bands from matching colors.
    if(nColumns % 2 == 1) {
        if(nColors % 2 == 0) {
            colWidth += 6;
        }
    } else {
        if(nColors % 2 == 1) {
            colWidth += 6;
        }
    }

}

bool RotRandColumns::step() {
    if (firstFrame) {
        for (int i = 0; i < canvas->getWidth(); i++) {
            uint8_t colorIndex = currColors[(i / colWidth) % nColors];
            canvas->fillColumn(i, colorIndex);
        }
        firstFrame = false;
    } else {
        canvas->rotateRight();
    }
    return true;
}
