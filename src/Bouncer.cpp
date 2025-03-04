#include "Bouncer.h"

Bouncer::Bouncer(Canvas* canvas, RGB color, int x, int y) : Animation(canvas), color(color), x(x), y(y) {
    maxX=canvas->getWidth() - 1;
    maxY=canvas->getHeight() - 1;
    up = true;
    right = true;
}

Bouncer::Bouncer(Canvas* canvas, RGB color, int x, int y, int maxX, int maxY) : Animation(canvas), color(color), x(x), y(y), maxX(maxX), maxY(maxY) {
    up = true;
    right = true;
}

bool Bouncer::step() {
    canvas->set(x, y, canvas->getBackground());
    if (up) {
        y++;
        if (y == maxY) {
            up = false;
        }
    } else {
        y--;
        if (y == 0) {
            up = true;
        }
    }

    if (right) {
        x++;
        if (x == maxX) {
            right = false;
        }
    } else {
        x--;
        if (x == 0) {
            right = true;
        }
    }

    canvas->set(x, y, color);
    return true;
}
