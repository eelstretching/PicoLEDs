#include "FireworkWipe.h"

#include "pico/printf.h"

FireworkWipe::FireworkWipe(Canvas* canvas) : Animation(canvas) {
    fw = new Firework*[canvas->getHeight()];
    for (int i = 0; i < canvas->getHeight(); i++) {
        fw[i] = new Firework(canvas, i);
        if (fw[i] == NULL) {
            printf("Null for %i!\n", i);
        }
        //
        // We want the wipe to last a little longer than the usual one.
        fw[i]->setMaxExplosionSteps(120);
    }
}

void FireworkWipe::init() {
    for (int i = 0; i < canvas->getHeight(); i++) {
        fw[i]->randomFlare();
        fw[i]->setState(State::START_EXPLOSION);
    }
}

bool FireworkWipe::step() {
    bool keepGoing = true;
    for (int i = 0; i < canvas->getHeight(); i++) {
        printf("FW %d step, %d %p\n", i, fw[i]->getState(), fw[i]);
        fw[i]->step();
        if (fw[i]->getState() == State::RESET) {
            //
            // If any of them tap out, call it a day.
            keepGoing = false;
        }
    }
    return keepGoing;
}
