#include "PacWipe.h"

#include "math8.h"

PacWipe::PacWipe(Canvas *canvas) : Animation(canvas) {
    sprites = new Sprite *[5];
    sprites[0] = new PacMan(canvas, 0, 1);
    sprites[1] = new Ghost(canvas, inkyColor, 0, 1);
    sprites[2] = new Ghost(*((Ghost *)sprites[1]), blinkyColor, 0, 1);
    sprites[3] = new Ghost(*((Ghost *)sprites[1]), pinkyColor, 0, 1);
    sprites[4] = new Ghost(*((Ghost *)sprites[1]), clydeColor, 0, 1);
}

void PacWipe::init() {
    curr = sprites[random8(0, 4)];
    if (curr == sprites[0]) {

        //
        // PacMan can only go right for now!
        curr->setDirection(RIGHT);
        curr->setStartPosition(1, 1);

    } else {
        if (random8(0, 100) < 50) {
            curr->setDirection(RIGHT);
            curr->setStartPosition(1, 1);
        } else {
            curr->setDirection(LEFT);
            curr->setStartPosition(canvas->getWidth() - 1, 1);
        }
    }
    curr->init();
}

bool PacWipe::step() { 
    bool ret = curr->step();
    if(curr->getDirection() == RIGHT) {
        canvas->clearColumn(curr->getX() - 1);
    } else {
        canvas->clearColumn(curr->getX() + curr->getWidth());
    }
    return ret;
}
