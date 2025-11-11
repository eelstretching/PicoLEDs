#include "Spiral.h"

Spiral::Spiral(Canvas* canvas, ColorMap *colorMap, int width, int length)
    : canvas(canvas), colorMap(colorMap), width(width), length(length) {}

Spiral::~Spiral() {}

void Spiral::init() {
    canvas->setColorMap(colorMap);
}

bool Spiral::step() {
    canvas->clear();
    for (int cx = x, c = 0; cx < x + width; cx++, c = (c+1) % canvas->getColorMap()->getSize()) {
        for (int cy = y, dx = cx; cy < y + length; cy++) {
            int mx = dx % canvas->getWidth();
            int my = cy % canvas->getHeight();
            canvas->set(mx, my, c);
            dx = (dx + 1) % canvas->getWidth();
        }
    }
    x = (x + 1) % canvas->getWidth();
    y = (y + 1) % canvas->getHeight();
    return true;
}