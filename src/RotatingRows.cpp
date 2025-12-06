#include "RotatingRows.h"

RotatingRows::RotatingRows(Canvas* canvas, ColorMap* colorMap, uint8_t nColors, uint8_t *colors,
                                 uint8_t rowWidth)
    : Animation(canvas, colorMap), rowWidth(rowWidth), nColors(nColors), colors(colors) {}

bool RotatingRows::step() {
    if (firstFrame) {
        for (int i = 0; i < canvas->getHeight(); i++) {
            uint8_t colorIndex = colors[(i / rowWidth) % nColors];
            canvas->fillRow(i, colorIndex);
        }
        firstFrame = false;
    } else {
        canvas->rotateUp();
    }
    return true;
}
