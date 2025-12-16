#include "Fireworks.h"

#include "pico/printf.h"

Fireworks::Fireworks(Canvas* canvas, ColorMap *colorMap) : Animation(canvas, colorMap, 60) {
    //
    // We'll make enough fireworks for the height of the canvas.
    nf = canvas->getHeight();
    fw = new Firework*[nf];
    for (int i = 0; i < nf; i++) {
        fw[i] = new Firework(canvas, colorMap, i);
        if (fw[i] == NULL) {
            printf("No firework for %i!\n", i);
        }
   }
}

Fireworks::Fireworks(Canvas* canvas, ColorMap *colorMap, Firework **fw, int nf) : Animation(canvas, colorMap), fw(fw), nf(nf) {
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
