#include "RotatingBars.h"

RotatingBars::RotatingBars(Canvas* canvas, ColorMap* colorMap, uint8_t barWidth,
                           uint8_t nColors)
    : Animation(canvas, colorMap),
      colorMap(colorMap),
      barWidth(barWidth),
      nColors(nColors) {}

bool RotatingBars::step() {
    if (firstFrame) {
        for (int i = 0; i < canvas->getWidth(); i++) {
            uint8_t colorIndex = (i / barWidth) % nColors;
            canvas->fillColumn(i, colorIndex);
        }
        firstFrame = false;
    } else {
        canvas->rotateRight();
    }
    return true;
}
