#include "Spiral.h"

Spiral::Spiral(Canvas* canvas, ColorMap* colorMap, uint8_t startx,
               uint8_t starty, uint8_t nColors, uint8_t* colors, uint8_t width,
               uint8_t length)
    : Animation(canvas, colorMap),
      startx(startx),
      starty(starty),
      nColors(nColors),
      colors(colors),
      width(width),
      length(length) {
    barWidth = length / nColors;
}

Spiral::~Spiral() {}

void Spiral::init() {
    canvas->setColorMap(colorMap);
    x = startx;
    y = starty;
}

bool Spiral::step() {
    int sx = x;
    int sy = y;
    if (clearPrev) {
        canvas->clear();
    }
    switch (direction) {
        case Direction::UP:
        case Direction::LEFT:
            for (int i = 0; i < length; i++) {
                uint8_t dy = sy + i;
                uint8_t colorIndex = colors[(i / barWidth) % nColors];
                for (int j = 0; j < width; j++) {
                    canvas->set((sx + j) % canvas->getWidth(),
                                dy % canvas->getHeight(), colorIndex);
                }
                sx++;
            }
            x = (x + 1) % canvas->getWidth();
            y = (y + 1) % canvas->getHeight();
            break;
        case Direction::DOWN:
        case Direction::RIGHT:
            for (int i = 0; i < length; i++) {
                int dy = sy - i;
                if (dy < 0) {
                    dy += canvas->getHeight();
                }
                uint8_t colorIndex = colors[(i / barWidth) % nColors];
                for (int j = 0; j < width; j++) {
                    canvas->set((sx + j) % canvas->getWidth(), dy, colorIndex);
                }
                sx++;
            }
            x = (x + 1) % canvas->getWidth();
            y--;
            if (y < 0) {
                y = canvas->getHeight() - 1;
            }
            break;
    }
    return true;
}