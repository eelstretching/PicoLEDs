#include "RotatingColumns.h"

RotatingColumns::RotatingColumns(Canvas* canvas, ColorMap* colorMap, uint8_t colWidth,
                           uint8_t nColors)
    : Animation(canvas, colorMap),
      colWidth(colWidth),
      nColors(nColors) {}

bool RotatingColumns::step() {
    if (firstFrame) {
        for (int i = 0; i < canvas->getWidth(); i++) {
            uint8_t colorIndex = (i / colWidth) % nColors;
            canvas->fillColumn(i, colorIndex);
        }
        firstFrame = false;
    } else {
        canvas->rotateRight();
    }
    return true;
}
