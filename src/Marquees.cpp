#include "Marquees.h"

Marquees::Marquees(Canvas* canvas, ColorMap *colorMap, int nColors, uint8_t *colors, int width, Direction direction,
                   int nMarquees)
    : Animation(canvas, colorMap),
      colors(colors),
      width(width),
      direction(direction),
      nMarquees(nMarquees) {
    switch (direction) {
        case UP:
        case DOWN:
            if (nMarquees < 0 || nMarquees > canvas->getWidth()) {
                nMarquees = canvas->getWidth();
            }
            marquees = new Marquee*[nMarquees];
            for (int i = 0, j = 0; i < nMarquees; i++, j+=2) {
                marquees[i] =
                    new Marquee(canvas, colorMap, nColors, colors, width, direction, i, i);
            }
            break;
        case LEFT:
        case RIGHT:
            if (nMarquees < 0 || nMarquees > canvas->getHeight()) {
                nMarquees = canvas->getHeight();
            }
            marquees = new Marquee*[nMarquees];
            for (int i = 0, j = 0; i < nMarquees; i++, j += 2) {
                marquees[i] =
                    new Marquee(canvas, colorMap, nColors, colors, width, direction, j, i);
            }
            break;
    }
}

Marquees::~Marquees() {
    for (int i = 0; i < nMarquees; i++) {
        delete marquees[i];
    }
    delete[] marquees;
}

bool Marquees::step() {
    for (int i = 0; i < nMarquees; i++) {
        marquees[i]->step();
    }
    return true;
}
