#include "RotatingColumns.h"

RotatingColumns::RotatingColumns(Canvas* canvas, ColorMap* colorMap,
                           uint8_t nColors, uint8_t *colors, uint8_t colWidth)
    : Animation(canvas, colorMap),
      colWidth(colWidth),
      colors(colors),
      nColors(nColors) {}

bool RotatingColumns::step() {
    if (firstFrame) {
        for (int i = 0; i < canvas->getWidth(); i++) {
            uint8_t colorIndex = colors[(i / colWidth) % nColors];
            canvas->fillColumn(i, colorIndex);
        }
        firstFrame = false;
    } else {
        canvas->rotateRight();
    }
    return true;
}
