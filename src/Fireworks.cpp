#include "Fireworks.h"

#include "pico/printf.h"

Fireworks::Fireworks(Canvas* canvas) : Animation(canvas) {
    //
    // We'll make enough fireworks for the height of the canvas.
    nf = canvas->getHeight();
    fw = new Firework*[nf];
    for (int i = 0; i < nf; i++) {
        fw[i] = new Firework(canvas, i);
        if (fw[i] == NULL) {
            printf("No firework for %i!\n", i);
        }
   }
}

Fireworks::Fireworks(Canvas* canvas, Firework **fw, int nf) : Animation(canvas), fw(fw), nf(nf) {
}

int getFPSNeeded() {
    return 60;
}

void Fireworks::init() {
    for (int i = 0; i < canvas->getHeight(); i++) {
        fw[i]->init();
    }
}

bool Fireworks::step() {
    for (int i = 0; i < canvas->getHeight(); i++) {
        fw[i]->step();
    }
    return true;
}

void Fireworks::finish() {
    canvas->clear();
}
