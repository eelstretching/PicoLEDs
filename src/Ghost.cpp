#include "Ghost.h"

Ghost::Ghost(Canvas* canvas, RGB color, int startX, int startY)
    : Sprite(canvas, startX, startY) {
    //
    // We get to create the pixmaps, others will copy these, to save precious
    // memory.
    xpms = new Xpm*[2];
    xpms[0] = new Xpm(ghost1);
    xpms[1] = new Xpm(ghost2);

    setup(color);
}

Ghost::Ghost(Ghost& o, RGB color, int startX, int startY)
    : Sprite(o.canvas, startX, startY) {
    //
    // We'll borrow the pimaps, and setup with our color.
    xpms = o.xpms;
    setup(color);
}

/// @brief Do the stuff that we need however we were created.
void Ghost::setup(RGB color) {
    //
    // The animation loop. Each state lasts for four frames
    add(xpms[0]);
    add(xpms[0]);
    add(xpms[0]);
    add(xpms[0]);
    add(xpms[1]);
    add(xpms[1]);
    add(xpms[1]);
    add(xpms[1]);

    //
    // Set up the color map that we'll render with based on the ghost color we
    // were given.
    RGB* gc = xpms[0]->getColors();
    int nc = xpms[0]->getNumberOfColors();
    colorMap = new RGB[nc];
    for (int i = 0; i < nc; i++) {
        colorMap[i] = gc[i];
    }
    colorMap[2] = color;
}

//
// We're going to assume that we're called after the animation step, at which
// point the pixels have been rendered into the canvas and now we need to add
// the pupil in the right place.
void Ghost::drawPupils() {
    //
    // figure out the bottom-left coordinate of the eye on the canvas.
    uint cx, cy;
    uint h = xpms[0]->getHeight();

    switch (direction) {
        case SPRITE_UP:
            cx = x + 2;
            cy = y + (h - 4) - 1;
            break;
        case SPRITE_DOWN:
            cx = x + 2;
            cy = y + (h - 7) - 1;
            break;
        case SPRITE_LEFT:
            cx = x + 1;
            cy = y + (h - 7) - 1;
            break;
        case SPRITE_RIGHT:
            cx = x + 4;
            cy = y + (h - 7) - 1;
            break;
    }

    //
    // Put the four pupil-colored pixels into each eye.
    canvas->set(cx, cy, pupil);
    canvas->set(cx + 6, cy, pupil);
    canvas->set(cx + 1, cy, pupil);
    canvas->set(cx + 7, cy, pupil);
    canvas->set(cx, cy + 1, pupil);
    canvas->set(cx + 6, cy + 1, pupil);
    canvas->set(cx + 1, cy + 1, pupil);
    canvas->set(cx + 7, cy + 1, pupil);
}

bool Ghost::step() {
    bool ret = Sprite::step();
    drawPupils();
    return ret;
}
