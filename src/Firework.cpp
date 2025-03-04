
#include "Firework.h"

#include <stdlib.h>

#include "color.h"
#include "colorpalettes.h"
#include "math8.h"
#include "pico.h"
#include "pico/platform.h"
#include "pico/printf.h"

static float constrain(float f, int min, int max) {
    if (f <= min) {
        return min;
    }
    if (f >= max) {
        return max;
    }
    return f;
}

Firework::Firework(Canvas* canvas, uint row) : Animation(canvas) {
    this->row = row;
    state = RESET;
    flare = new Spark[flareSize];
    explosion = new Spark[canvas->getWidth() / EXPLOSION_DIVISOR];
}

void Firework::reset() {
    //
    // Set the flare to trail along.
    for (int i = 0; i < flareSize; i++) {
        //
        // The particles in the flare have their velocity based on
        // the previous one in the flare, each one's a little slower than the
        // previous one. The color's based on the speed.
        flare[i].pos = 0;
        if (i == 0) {
            flare[i].vel = float(random16(80, 90)) / 100;
        } else {
            flare[i].vel = 0.95 * flare[i - 1].vel;
        }
        flare[i].val = constrain(flare[i].vel * 1000, 0, 255);
    }
    //
    // After reset, we'll pause for a bit.
    pauseTime = random8(10, 90);
    state = PAUSE;
    canvas->clearRow(row);
}

void Firework::rise() {
    canvas->clearRow(row);

    //
    // If any of our flare sparks stop, then it's time to go boom.
    for (int i = 0; i < flareSize; i++) {
        if (flare[i].vel <= 0) {
            state = START_EXPLOSION;
            return;
        }
    }

    //
    // The position increases according to the velocity, and the velocity
    // decreases according to gravity.
    //
    // Trail of sparks behind the rising flare.
    for (int i = 0; i < flareSize; i++) {
        Spark* s = &flare[i];
        s->pos = constrain(s->pos + s->vel, 0, canvas->getWidth());
        s->vel -= gravity;
        s->val = constrain(s->vel * 1000, 0, 255);
        RGB sc = getFlareColor(s->val);
        canvas->set(s->pos, row, sc);
    }
}

void Firework::randomFlare() {
    flare[0].pos = canvas->getWidth() / 2;
    int rand = random8(4);
    if (random8(10) < 5) {
        flare[0].pos -= rand;
    } else {
        flare[0].pos += rand;
    }
}

void Firework::halfFlare() { flare[0].pos = canvas->getWidth() / 2; }

void Firework::startExplosion() {
    //
    // We want a number of sparks proportional to the position of the head of
    // the flare. This looks about right.
    numSparks = MAX(canvas->getWidth() / EXPLOSION_DIVISOR,
                    flare[0].pos / EXPLOSION_DIVISOR);
    explosionSteps = 0;

    // initialize sparks
    for (int i = 0; i < numSparks; i++) {
        Spark* s = &explosion[i];
        s->pos = flare[0].pos;
        // Velocity from -1 to 1, which means some go "up" and some go "down".
        s->vel = (float(random16(0, 10000)) / 5000.0) - 1.0;
        //
        // Goose the ones going up, so they go a bit higher.
        if (s->vel > 0) {
            s->vel *= 1.4;
        }
        // set colors before scaling velocity to keep them bright
        s->val = constrain(abs(s->vel) * 500, 0, 255);
        // proportional to height
        s->vel *= s->pos / canvas->getWidth();
    }
    dyingGravity = gravity;
    c1 = 120;
    c2 = 50;
    state = EXPLODING;
}

RGB Firework::getFlareColor(uint val) {
    RGB sc = HeatColor(val);
    sc %= 50;
    return sc;
}

RGB Firework::getColor(float val, uint c1, uint c2) {
    if (val > c1) {
        return RGB(255, 255, (255 * (val - c1)) / (255 - c1));
    } else if (val < c2) {  // fade from red to black
        return RGB((255 * val) / c2, 0, 0);
    } else {  // fade from yellow to red
        return RGB(255, (255 * (val - c2)) / (c1 - c2), 0);
    }
}

void Firework::explode() {
    if (explosion[0].val <= c2 / 128 || explosionSteps >= maxExplosionSteps) {
        state = RESET;
        return;
    }

    for (int i = 0; i < numSparks; i++) {
        Spark* s = &explosion[i];
        s->pos += s->vel;
        s->pos = constrain(s->pos, 0, canvas->getWidth());
        s->vel -= dyingGravity;
        s->val = constrain(s->val * 0.99, 0, 255);
        canvas->set(int(s->pos), row, getColor(s->val, c1, c2));
    }
    //
    // As sparks burn out they fall slower
    dyingGravity *= .995;
    explosionSteps++;
}

bool Firework::step() {
    switch (state) {
        case RESET:
            reset();
            break;
        case PAUSE:
            if (pauseTime == 0) {
                state = RISING;
            }
            pauseTime--;
            break;
        case RISING:
            rise();
            break;
        case START_EXPLOSION:
            startExplosion();
            break;
        case EXPLODING:
            explode();
            break;
    }
    //
    // By default, we're a never ending animation.
    return false;
}

RGB PaletteFirework::getFlareColor(uint val) {
    return ColorFromPalette(*palette, val);
}

RGB PaletteFirework::getColor(float val, uint c1, uint c2) {
    return ColorFromPalette(*palette, val);
}
