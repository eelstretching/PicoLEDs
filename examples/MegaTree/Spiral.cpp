#include "Spiral.h"

Spiral::Spiral(Canvas* canvas, RGB* color, int width, int length)
    : Animation(canvas), colors(color), width(width), length(length) {}

Spiral::~Spiral() {}

bool Spiral::step() {
    canvas->clear();
    for (int cx = x, c = 0; cx < x + width; cx++, c++) {
        for (int cy = y, dx = cx, f = 0; cy < y + length; cy++, f++) {
            int mx = dx % canvas->getWidth();
            int my = cy % canvas->getHeight();
            canvas->set(mx, my, colors[c]);
            switch (f) {
                case 0:
                    canvas->fade(mx, my, 16);
                    break;
                case 1:
                    canvas->fade(mx, my, 8);
                    break;
            }
            dx = (dx + 1) % canvas->getWidth();
        }
    }
    x = (x + 1) % canvas->getWidth();
    y = (y + 1) % canvas->getHeight();
    return true;
}