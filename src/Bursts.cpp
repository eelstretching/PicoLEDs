#include "Bursts.h"

#include "ArrayColorMap.h"

void Burst::init() {
    state = BurstState::WAITING;
    p = 0;
    speed = random8(3) + 1;
}

bool Burst::step() {
    switch (state) {
        case WAITING:
            if (random16(0, 100) > 50) {
                state = BurstState::BURSTING;
                return true;
            }
            break;
        case BURSTING:
            if (p >= canvas->getWidth()) {
                state = BurstState::WAITING;
                p = 0;
                return true;
            }
            canvas->fillColumn(p, 0);
            for (int i = 0, x = p - 1; i < tailLen && x >= 0; i++, x--) {
                canvas->fillColumn(x, i + 1);
            }
            p += speed;
            break;
    }
    return true;
}

Bursts::Bursts(Canvas* canvas, RGB backGround, uint8_t maxBursts,
               RGB burstColor, uint8_t tailLen)
    : Animation(canvas, nullptr), maxBursts(maxBursts), tailLen(tailLen) {
    colorMap = new ArrayColorMap(16);
    colorMap->setBackground(backGround);
    //
    // The tails of the bursts will fade out over a few pixels.
    for (int i = 0; i < colorMap->getSize(); i++) {
        colorMap->addColor(burstColor.fadeToBlackBy(64));
    }
    bursts = new Burst*[maxBursts];
    for (int i = 0; i < maxBursts; i++) {
        bursts[i] = new Burst(canvas, tailLen + random8(4));
    }
}

Bursts::~Bursts() {
    for (int i = 0; i < maxBursts; i++) {
        delete bursts[i];
    }
    delete bursts;
}

void Bursts::init() {
    canvas->setColorMap(colorMap);
    int nBursts = MAX(random8(maxBursts) + 2, maxBursts);
    for (int i = 0; i < nBursts; i++) {
        bursts[i]->init();
    }
}

bool Bursts::step() {
    canvas->clear();
    for (int i = 0; i < nBursts; i++) {
        bursts[i]->step();
    }
    return true;
}
