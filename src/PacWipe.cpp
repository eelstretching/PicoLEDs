#include "PacWipe.h"

#include "math8.h"

PacWipe::PacWipe(Canvas* canvas, ColorMap* colorMap)
    : Animation(canvas, colorMap) {
    ghostFrames[0] = new Xpm(ghost1);
    ghostFrames[1] = new Xpm(ghost2);

    sprites = new Sprite*[5];
    sprites[0] = new PacMan(canvas, 0, 1);
    sprites[1] = new Ghost(canvas, ghostFrames, inkyColor,
                           pupilColor, 0, 1, Direction::RIGHT);
    sprites[2] = new Ghost(canvas, ghostFrames, blinkyColor,
                           pupilColor, 0, 1, Direction::RIGHT);
    sprites[3] = new Ghost(canvas, ghostFrames, pinkyColor,
                           pupilColor, 0, 1, Direction::RIGHT);
    sprites[4] = new Ghost(canvas, ghostFrames, clydeColor,
                           pupilColor, 0, 1, Direction::RIGHT);
}

void PacWipe::init() {
    curr = sprites[random8(0, 4)];
    if (curr == sprites[0]) {
        //
        // PacMan can only go right for now!
        curr->setDirection(Direction::RIGHT);
        curr->setStartPosition(1, 1);

    } else {
        if (random8(0, 100) < 50) {
            curr->setDirection(Direction::RIGHT);
            curr->setStartPosition(1, 1);
        } else {
            curr->setDirection(Direction::LEFT);
            curr->setStartPosition(canvas->getWidth() - 1, 1);
        }
    }
    curr->init();
}

bool PacWipe::step() {
    bool ret = curr->step();
    if (curr->getDeltaX() > 0) {
        canvas->clearColumn(curr->getX() - 1);
    } else {
        canvas->clearColumn(curr->getX() + curr->getWidth());
    }
    return ret;
}
