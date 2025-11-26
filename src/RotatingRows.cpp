#include "RotatingRows.h"

RotatingRows::RotatingRows(Canvas* canvas, ColorMap* colorMap,
                                 uint8_t rowWidth, uint8_t nColors)
    : Animation(canvas, colorMap), rowWidth(rowWidth), nColors(nColors) {}

bool RotatingRows::step() {
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
