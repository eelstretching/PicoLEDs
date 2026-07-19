#include "PacMan.h"

#include "Direction.h"

PacMan::PacMan(Canvas* canvas, int startX, int startY)
    : Sprite(canvas, startX, startY, Direction::RIGHT) {
    frames[0] = new Xpm(pac1);
    frames[1] = new Xpm(pac2);
    frames[2] = new Xpm(pac3);

    //
    // Define the animation loop. We'll have each state last for two frames.
    add(frames[0]);
    add(frames[0]);
    add(frames[1]);
    add(frames[1]);
    add(frames[2]);
    add(frames[2]);
    add(frames[1]);
    add(frames[1]);
}

PacMan::~PacMan() {
    for(int i=0; i<3; i++) {
        delete frames[i];
    }
}
