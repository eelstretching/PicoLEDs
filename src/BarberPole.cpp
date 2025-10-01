#include "BarberPole.h"

void BarberPole::init() {
    //
    // Stripes will be diagonal, so we need to start back far enough to fill
    // the entire canvas. This will be off the canvas, but canvas will be
    // OK with that.
    start = 0;
}

bool BarberPole::step() {
    //
    // We're going to draw diagonals across the entire canvas, alternating
    // colors every stripeWidth stripes.
    int ci = 0;
    for (int i = 0; i < canvas->getWidth(); i++) {
        //
        // We'll start drawing at start and let the stripes wrap around the
        // end, so we'll use modulo arithmetic to manage that.
        int x = (start + i) % canvas->getWidth();

        //
        // For each x, we'll draw the diagonal fully, wrapping around on the x
        // axis if necessary.
        for (int y = 0; y < canvas->getHeight(); y++) {
            canvas->set(x % canvas->getWidth(), y, ci);
            x++;
        }
        if (i % stripeWidth == 0) {
            //
            // Switch colors at the stripe width.
            ci = (ci + 1) % nColors;
        }
    }
    //
    // Next time, we'll start one over from here.
    start = (start+1) % canvas->getWidth();
    return true;
}
