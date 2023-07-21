#include "RainbowWipe.h"

#include "colorutils.h"
#include "math8.h"
#include "pico/printf.h"

void RainbowWipe::init() {
    if (random8(100) < 50) {
        direction = LEFT;
        p = canvas->getWidth() - 2;
    } else {
        direction = RIGHT;
        p = 1;
    }
    clearing = false;
}

bool RainbowWipe::step() {
    if (clearing) {
        if (p < 0) {
            return false;
        }
        canvas->clearRow(p--);
        return true;
    }

    //
    // Rainbow for each row.
    int n;
    switch (direction) {
        case LEFT:
            if (p < 0) {
                p = canvas->getHeight() - 1;
                clearing = true;
                return true;
            }
            n = canvas->getWidth() - p;
            fill_rainbow(canvas->getDataPointer(p, 0), n, 0, 7);
            for (int row = 1; row < canvas->getHeight(); row++) {
                canvas->copy(canvas->getDataPointer(p, 0), n, p, row);
            }
            p--;
            break;

        case RIGHT:
            if (p >= canvas->getWidth()) {
                p = canvas->getHeight() - 1;
                clearing = true;
                return true;
            }
            fill_rainbow(canvas->getDataPointer(0, 0), p, 0, 7);
            for (int row = 1; row < canvas->getHeight(); row++) {
                canvas->copy(canvas->getDataPointer(0, 0), p, 0, row);
            }
            p++;
            break;
    }

    return true;
}
