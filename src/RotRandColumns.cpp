#include "RotRandColumns.h"

RotRandColumns::RotRandColumns(Canvas* canvas, ColorMap* colorMap)
    : Animation(canvas, colorMap), colWidth(colWidth), nColors(nColors) {}

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
    colWidth = random16(5, canvas->getWidth() / 4);
    if (canvas->getWidth() % colWidth == 0) {
        colWidth++;
    }
    printf("colWidth: %d ", colWidth);
    printf("colors: ");
    for (int i = 0; i < nColors; i++) {
        printf("%d ", currColors[i]);
    }
    printf("\n");
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
