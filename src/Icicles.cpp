#include "Icicles.h"

Drip::Drip(Canvas* canvas, uint pos, uint length, uint tailLen,
           Direction direction)
    : Animation(canvas),
      dp(pos),
      length(length),
      tailLen(tailLen),
      direction(direction) {}

void Drip::init() {
    switch (direction) {
        case LEFT:
            dp = canvas->getWidth() - 1;
            break;
        case RIGHT:
            dp = 0;
            break;
        case UP:
            dp = 0;
            break;
        case DOWN:
            dp = canvas->getHeight() - 1;
            break;
    }
    state = DRIPPING;
    speed = 0.01;
}

void Drip::setPos(uint pos) { this->pos = pos; }

bool Drip::step() {
    switch (state) {
        case DRIPPING:

            //
            // 60/40 chance of moving down a notch.
            if (random16(100) < 60) {
                dp++;
            }

            if (dp >= length) {
                state = FALLING;
                return true;
            }
            //
            // Draw the drip with the brightest color.
            switch (direction) {
                case LEFT:
                    canvas->set(canvas->getWidth() - dp, pos, 0);
                    break;
                case RIGHT:
                    canvas->set(dp, pos, 0);
                    break;
                case UP:
                    canvas->set(pos, dp, 0);
                    break;
                case DOWN:
                    canvas->set(pos, canvas->getHeight() - dp, 0);
                    break;
            }
            break;
        case FALLING:

            //
            // Move the drip down faster.
            dp += speed;
            switch (direction) {
                case LEFT:
                    if (dp < 0) {
                        state = DRIPPING;
                        init();
                        return true;
                    }
                    canvas->set(dp, pos, 0);
                    for (int i = 0; i < tailLen; i++) {
                        canvas->set(dp + i + 1, pos, i + 1);
                    }
                    break;
                case RIGHT:
                    if (dp >= canvas->getWidth()) {
                        state = DRIPPING;
                        init();
                        return true;
                    }
                    canvas->set(dp, pos, 0);
                    for (int i = 0; i < tailLen; i++) {
                        canvas->set(dp - i + 1, pos, i + 1);
                    }
                    break;
                case UP:
                    if (dp >= canvas->getHeight()) {
                        state = DRIPPING;
                        init();
                        return true;
                    }
                    canvas->set(pos, dp, 0);
                    for (int i = 0; i < tailLen; i++) {
                        canvas->set(pos, dp - i + 1, i + 1);
                    }
                    break;
                case DOWN:
                    if (dp < 0) {
                        state = DRIPPING;
                        init();
                        return true;
                    }
                    canvas->set(pos, dp, 0);
                    for (int i = 0; i < tailLen; i++) {
                        canvas->set(pos, dp + i + 1, i + 1);
                    }
                    break;
            }

            switch (direction) {
                case LEFT:
                case DOWN:
                    if (speed > -MAX_SPEED) {
                        speed -= 0.1;
                    } else {
                        speed = -MAX_SPEED;
                    }
                    break;
                case UP:
                case RIGHT:
                    if (speed < MAX_SPEED) {
                        speed += 0.1;
                    } else {
                        speed = MAX_SPEED;
                    }
                    break;
            }
            break;
    }
    return true;
}

Icicle::Icicle(Canvas* canvas, uint pos, uint length, Direction direction,
               RGB color)
    : Animation(canvas), pos(pos), length(length), direction(direction) {
    drip = new Drip(canvas, pos, length, 5, direction);
    //
    // Create a color map that goes from the brightest color to the dimmest.
    colorMap->addColor(color);
    //
    // Ripping off fadeToBlack from FastLED, but for colors.
    for (int i = 0; i < colorMap->getSize(); i++) {
        colorMap->addColor(color.nscale8(235));
    }
    canvas->setColorMap(colorMap);
}

void Icicle::setPos(uint pos) {
    this->pos = pos;
    drip->setPos(pos);
}

void Icicle::init() { drip->init(); }

bool Icicle::step() {
    //
    // Draw the icicle
    for (uint i = 0; i < length; i++) {
        switch (direction) {
            case LEFT:
                canvas->set(canvas->getWidth() - i, pos, 2);
                break;
            case RIGHT:
                canvas->set(0 + i, pos, 2);
                break;
            case UP:
                canvas->set(pos + i, 0, 2);
                break;
            case DOWN:
                canvas->set(pos, canvas->getHeight() - i, 2);
                break;
        }
    }
    drip->step();
    return true;
}

Icicles::Icicles(Canvas* canvas, uint numIcicles, uint length, RGB color)
    : Animation(canvas), numIcicles(numIcicles) {
    icicles = new Icicle*[numIcicles];

    for (uint i = 0; i < numIcicles; i++) {
        icicles[i] =
            new Icicle(canvas, 0, length + random16(0, 6), LEFT, color);
    }
}

void Icicles::init() {
    printf("Initializing %d icicles\n", numIcicles);
    for (int i = 0; i < numIcicles; i++) {
        icicles[i]->setPos(canvas->getHeight() + 1);
    }
    for (int i = 0; i < numIcicles; i++) {
        printf("Icicle %d\n", i);
        bool made = false;
        uint pos = 0;
        while (!made) {
            pos = random16(0, canvas->getHeight());
            if (i == 0) {
                icicles[i]->setPos(pos);
                made = true;
            } else {
                for (int j = 0; j < i; j++) {
                    if (icicles[j]->pos == pos) {
                        break;
                    } else {
                        made = true;
                    }
                }
            }
        }
        printf("Icicle %d at pos %d\n", i, pos);
        icicles[i]->setPos(pos);
        icicles[i]->init();
    }
}

Icicles::~Icicles() {
    for (uint i = 0; i < numIcicles; i++) {
        delete icicles[i];
    }
    delete[] icicles;
}

bool Icicles::step() {
    canvas->clear();
    for (uint i = 0; i < numIcicles; i++) {
        icicles[i]->step();
    }
    return true;
}
