#include "PacMan.h"

PacMan::PacMan(Canvas *canvas, int startX, int startY) : Sprite(canvas, startX, startY) {

    //
    // Define the animation loop. We'll have each state last for two frames.
    add(&xpms[0]);
    add(&xpms[0]);
    add(&xpms[1]);
    add(&xpms[1]);
    add(&xpms[2]);
    add(&xpms[2]);
    add(&xpms[1]);
    add(&xpms[1]);
    setDirection(SPRITE_RIGHT);
}