#include "Spiral.h"

Spiral::Spiral(Canvas* canvas, RGB* color, int width, int length)
    : canvas(canvas), color(color), width(width), length(length) {}

Spiral::~Spiral() {}

bool Spiral::step() {
    canvas->clear();
    for (int cx = x, c = 0; cx < x + width; cx++, c++) {
        for (int cy = y, dx = cx; cy < y + length; cy++) {
            canvas->set(dx % canvas->getWidth(), cy % canvas->getHeight(),
                        color[c]);
            dx = (dx + 1) % canvas->getWidth();
        }
    }
    x = (x + 1) % canvas->getWidth();
    y = (y + 1) % canvas->getHeight();
    return true;
}