#include "Marquee.h"

bool Marquee::step() {
    int p = pos;
    switch (direction) {
        case LEFT:
            for (int i = 0; i < canvas->getWidth(); i++) {
                canvas->set(p, coord, colors[(i / width) % nColors]);
                p--;
                if(p < 0) {
                    p = canvas->getWidth() - 1;
                }
            }
            pos--;
            break;
        case RIGHT:
            for (int i = 0; i < canvas->getWidth(); i++) {
                canvas->set(p, coord, colors[(i / width) % nColors]);
                p = (p + 1) % canvas->getWidth();
            }
            pos++;
            break;
        case UP:
            for (int i = 0; i < canvas->getHeight(); i++) {
                canvas->set(coord, p, colors[(i / width) % nColors]);
                p = (p + 1) % canvas->getHeight();
            }
            pos++;
            break;
        case DOWN:
            for (int i = 0; i < canvas->getHeight(); i++) {
                canvas->set(coord, p, colors[(i / width) % nColors]);
                p--;
                if(p < 0) {
                    p = canvas->getHeight() - 1;
                }
            }
            pos--;
            break;
    }
    return true;
}
