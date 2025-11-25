#include "PacMan.h"

#include "Direction.h"

PacMan::PacMan(Canvas* canvas, ColorMap *colorMap, int startX, int startY)
    : Sprite(canvas, colorMap, startX, startY) {
    xpms = new Xpm*[3];
    xpms[0] = new Xpm(pac1, colorMap);
    xpms[1] = new Xpm(pac2, colorMap);
    xpms[2] = new Xpm(pac3, colorMap);

    //
    // Define the animation loop. We'll have each state last for two frames.
    add(xpms[0]);
    add(xpms[0]);
    add(xpms[1]);
    add(xpms[1]);
    add(xpms[2]);
    add(xpms[2]);
    add(xpms[1]);
    add(xpms[1]);
    setDirection(RIGHT);
}

PacMan::~PacMan() {
    for(int i=0; i<3; i++) {
        delete xpms[i];
    }
    delete[] xpms;
}
