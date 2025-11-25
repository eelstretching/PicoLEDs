#include "FireworkWipe.h"

#include "pico/printf.h"

FireworkWipe::FireworkWipe(Canvas* canvas, ColorMap *colorMap) : Animation(canvas, colorMap) {
    //
    // We'll make enough fireworks for the height of the canvas.
    nf = canvas->getHeight();
    fw = new Firework*[nf];
    for (int i = 0; i < nf; i++) {
        fw[i] = new Firework(canvas, colorMap, i);
        if (fw[i] == NULL) {
            printf("No firework for %i!\n", i);
        }
        //
        // We want the wipe to last a little longer than the usual one.
        fw[i]->setMaxExplosionSteps(140);
    }
}

FireworkWipe::FireworkWipe(Canvas* canvas, ColorMap *colorMap, Firework **fw, int nf) : Animation(canvas, colorMap), fw(fw), nf(nf) {
    for (int i = 0; i < nf; i++) {
        //
        // We want the wipe to last a little longer than the usual one.
        fw[i]->setMaxExplosionSteps(140);
    }
}

void FireworkWipe::init() {
    for (int i = 0; i < canvas->getHeight(); i++) {
        fw[i]->halfFlare();
        fw[i]->setState(FillState::START_EXPLOSION);
    }
}

bool FireworkWipe::step() {
    bool keepGoing = true;
    for (int i = 0; i < canvas->getHeight(); i++) {
        fw[i]->step();
        if (fw[i]->getState() == FillState::RESET) {
            //
            // If any of them tap out, call it a day.
            keepGoing = false;
        }
    }
    return keepGoing;
}

void FireworkWipe::finish() {
    canvas->clear();
}
