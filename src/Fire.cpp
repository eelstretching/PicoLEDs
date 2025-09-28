#include "Fire.h"

#include <stdlib.h>
#include <string.h>

#include "math8.h"
#include "pico.h"
#include "pico/platform.h"

Fire::Fire(Canvas *canvas, 
            uint x, 
            uint n, 
            uint row, 
            uint cooling,
           uint sparking)
    : canvas(canvas),
      x(x),
      n(n),
      row(row),
      cooling(cooling),
      sparking(sparking) {
    end = MIN(x + n, canvas->getWidth());
    heat = (uint8_t *)malloc(n * sizeof(uint8_t));
    memset(heat, 0, n * sizeof(uint8_t));
}

Fire::~Fire() { free(heat); }

bool Fire::step() {
    aw.start();
    for (int i = 0; i < n; i++) {
        heat[i] = qsub8(heat[i], random8(0, ((cooling * 10) / n) + 2));
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for (int k = n - 1; k >= 2; k--) {
        heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if (random8() < sparking) {
        int y = random8(7);
        heat[y] = qadd8(heat[y], random8(160, 255));
    }

    // Step 4.  Map from heat cells to LED colors
    for (int j = 0; j < n; j++) {
        canvas->set(x + j, row, heat[j]);
    }
    aw.finish();
    return true;
};