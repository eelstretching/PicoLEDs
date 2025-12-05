#include "PacWipe.h"

#include "math8.h"

PacWipe::PacWipe(Canvas* canvas, ColorMap* colorMap)
    : Animation(canvas, colorMap) {
    uint8_t pupilColorIndex = colorMap->addColor(pupilColor);
    uint8_t pacColorIndex = colorMap->addColor(pacColor);

    ghostFrames[0] = new Xpm(ghost1);
    ghostFrames[1] = new Xpm(ghost2);

    sprites = new Sprite*[5];
    sprites[0] = new PacMan(canvas, pacColorIndex, 0, 1);
    sprites[1] = new Ghost(canvas, ghostFrames, colorMap->addColor(inkyColor),
                           pupilColorIndex, 0, 1);
    sprites[2] = new Ghost(canvas, ghostFrames, colorMap->addColor(blinkyColor),
                           pupilColorIndex, 0, 1);
    sprites[3] = new Ghost(canvas, ghostFrames, colorMap->addColor(pinkyColor),
                           pupilColorIndex, 0, 1);
    sprites[4] = new Ghost(canvas, ghostFrames, colorMap->addColor(clydeColor),
                           pupilColorIndex, 0, 1);
}

void PacWipe::init() {
    canvas->setColorMap(colorMap);
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
    if (curr->getDirection() == RIGHT) {
        canvas->clearColumn(curr->getX() - 1);
    } else {
        canvas->clearColumn(curr->getX() + curr->getWidth());
    }
    return ret;
}
