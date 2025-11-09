#include "ColorCone.h"

ColorCone::ColorCone(Canvas* canvas, ColorMap *colorMap) : Animation(canvas), colorMap(colorMap) {
    bandSize = canvas->getWidth() / colorMap->getUsed();
}

void ColorCone::init() {
    buildDone = false;
    currCol = 0;
    currColor = 0;
    canvas->setColorMap(colorMap);
    canvas->clear();
}

bool ColorCone::step() {
    if (!buildDone) {
        //
        // Build up the cone column by column.
        canvas->fillColumn(currCol, currColor);
        currCol++;
        if (currCol >= canvas->getWidth()) {
            buildDone = true;
        }
        if (currCol % bandSize == 0) {
            currColor = (currColor + 1) % colorMap->getUsed();
        }
    }
    return true;
}