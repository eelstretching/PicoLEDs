
#include "Firework.h"

#include <stdlib.h>

#include "color.h"
#include "colorpalettes.h"
#include "math8.h"
#include "pico.h"
#include "pico/platform.h"

static float constrain(float f, int min, int max) {
    if (f <= min) {
        return min;
    }
    if (f >= max) {
        return max;
    }
    return f;
}

Firework::Firework(Canvas* canvas, uint row)
    : canvas(canvas), row(row), numSparks(numSparks) {
    state = RESET;
    flare = new Spark[flareSize];
    explosion = NULL;
}

void Firework::reset() {
    //
    // Set the initial position and velocity for the head of the flare.
    flare->pos = 0;
    flare->vel = float(random16(70, 90)) / 100;
    brightness = 1;

    for (int i = 1; i < flareSize; i++) {
        //
        // The particles in the tail of the flare have their velocity based on
        // the velocity of the head, each one's a little slower than the
        // previous one. The color's based on the speed.
        flare[i].pos = 0;
        flare[i].vel = 0.95 * flare[i - 1].vel;
        flare[i].val = constrain(flare[i].vel * 1000, 0, 255);
    }
    canvas->clearRow(row);
    state = RISING;
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
    canvas->set(int(flare[0].pos), row, RGB(HSV(0, 0, int(brightness * 255))));
    flare[0].pos += flare[0].vel;
    flare[0].vel -= gravity;

    //
    // Trail of sparks behind the rising flare.
    for (int i = 1; i < flareSize; i++) {
        Spark* s = &flare[i];
        s->pos = constrain(s->pos + s->vel, 0, canvas->getWidth());
        s->vel -= gravity;
        s->val = constrain(s->vel * 1000, 0, 255);
        RGB sc = HeatColor(s->val);
        sc %= 50;
        canvas->set(s->pos, row, sc);
    }
    brightness *= 0.985;
}

void Firework::startExplosion() {
    //
    // We want a number of sparks proportional to the position of the head of
    // the flare. This looks about right.
    numSparks = flare[0].pos / 2;
    if (explosion != NULL) {
        delete[] explosion;
    }
    explosion = new Spark[numSparks];
    explosionSteps = 0;

    // initialize sparks
    for (int i = 0; i < numSparks; i++) {
        Spark* s = &explosion[i];
        s->pos = flare[0].pos;
        // Velocity from -1 to 1, which means some go "up" and some go "down".
        s->vel = (float(random16(0, 20000)) / 10000.0) - 1.0;
        // set colors before scaling velocity to keep them bright
        s->val = constrain(abs(s->vel) * 500, 0, 255);
        s->vel *= s->pos / canvas->getWidth();  // proportional to height
    }
    explosion[0].val = 255;  // this will be our known spark
    dyingGravity = gravity;
    c1 = 120;
    c2 = 50;
    state = EXPLODING;
}

void Firework::explode() {
    if (explosion[0].val <= c2 / 128 || explosionSteps >= maxExplosionSteps) {
        state = RESET;
        return;
    }

    canvas->clearRow(row);
    for (int i = 0; i < numSparks; i++) {
        Spark* s = &explosion[i];
        s->pos += s->vel;
        s->pos = constrain(s->pos, 0, canvas->getWidth());
        s->vel -= dyingGravity;
        s->val = constrain(s->val * 0.99, 0, 255);
        if (s->val > c1) {
            canvas->set(int(s->pos), row,
                        RGB(255, 255, (255 * (s->val - c1)) / (255 - c1)));
        } else if (s->val < c2) {  // fade from red to black
            canvas->set(int(s->pos), row, RGB((255 * s->val) / c2, 0, 0));
        } else {  // fade from yellow to red
            canvas->set(int(s->pos), row,
                        RGB(255, (255 * (s->val - c2)) / (c1 - c2), 0));
        }
    }
    //
    // As sparks burn out they fall slower
    dyingGravity *= .995;
    explosionSteps++;
}

void Firework::step() {
    switch (state) {
        case RESET:
            reset();
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
}
