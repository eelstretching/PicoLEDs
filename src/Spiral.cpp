#include "Spiral.h"

Spiral::Spiral(Canvas* canvas, ColorMap* colorMap, uint8_t nColors,
               uint8_t* colors, uint8_t width, uint8_t length)
    : Animation(canvas, colorMap),
      nColors(nColors),
      colors(colors),
      width(width),
      length(length) {
    barWidth = length / nColors;
}

Spiral::~Spiral() {}

void Spiral::init() {
    canvas->setColorMap(colorMap);
    x = 0;
    y = 0;
}

bool Spiral::step() {
    int sx = x;
    int sy = y;
    canvas->clear();
    for (int i = 0; i < length; i++) {
        uint8_t dy = sy + i;
        uint8_t colorIndex = colors[(i / barWidth) % nColors];
        for (int j = 0; j < width; j++) {
            canvas->set((sx+j) % canvas->getWidth(), dy % canvas->getHeight(),
                        colorIndex);
        }
        sx++;
    }
    x = (x + 1) % canvas->getWidth();
    y = (y + 1) % canvas->getHeight();
    return true;
}