#include "Ghost.h"

Ghost::Ghost(Canvas* canvas, Xpm **frames, uint8_t ghostColorIndex, uint8_t pupilColorIndex, int startX, int startY)
    : Sprite(canvas, colorMap, startX, startY), frames(frames) {

    //
    // The animation loop. Each state lasts for four frames
    add(frames[0]);
    add(frames[0]);
    add(frames[0]);
    add(frames[0]);
    add(frames[1]);
    add(frames[1]);
    add(frames[1]);
    add(frames[1]);

    cmap[0] = 0;
    cmap[1] = ghostColorIndex;
    cmap[2] = pupilColorIndex;
}

Ghost::~Ghost() {
}

//
// We're going to assume that we're called after the animation step, at which
// point the pixels have been rendered into the canvas and now we need to add
// the pupil in the right place.
void Ghost::drawPupils() {
    //
    // figure out the bottom-left coordinate of the eye on the canvas.
    uint cx, cy;
    uint h = frames[0]->getHeight();

    switch (direction) {
        case UP:
            cx = x + 2;
            cy = y + (h - 4) - 1;
            break;
        case DOWN:
            cx = x + 2;
            cy = y + (h - 7) - 1;
            break;
        case LEFT:
            cx = x + 1;
            cy = y + (h - 7) - 1;
            break;
        case RIGHT:
            cx = x + 4;
            cy = y + (h - 7) - 1;
            break;
    }

    //
    // Put the four pupil-colored pixels into each eye.
    canvas->set(cx, cy, cmap[2]);
    canvas->set(cx + 6, cy, cmap[2]);
    canvas->set(cx + 1, cy, cmap[2]);
    canvas->set(cx + 7, cy, cmap[2]);
    canvas->set(cx, cy + 1, cmap[2]);
    canvas->set(cx + 6, cy + 1, cmap[2]);
    canvas->set(cx + 1, cy + 1, cmap[2]);
    canvas->set(cx + 7, cy + 1, cmap[2]);
}

bool Ghost::step() {
    bool ret = Sprite::step();
    drawPupils();
    return ret;
}
