#include "Ghost.h"

Ghost::Ghost(Canvas* canvas, ColorMap *colorMap, RGB &ghostColor, int startX, int startY)
    : Sprite(canvas, colorMap, startX, startY) {
    //
    // We get to create the pixmaps, others will copy these, to save precious
    // memory.
    xpms = new Xpm*[2];
    xpms[0] = new Xpm(ghost1, canvas->getColorMap());
    xpms[1] = new Xpm(ghost2, canvas->getColorMap());

    setup(ghostColor);
}

Ghost::~Ghost() {
    for(int i=0; i<2; i++) {
        delete xpms[i];
    }
    delete[] xpms;
}

/// @brief Do the stuff that we need however we were created.
void Ghost::setup(RGB &ghostColor) {
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
    // Add the ghost color to the color map and remember its index.
    uint8_t ghostColorIndex = xpms[0]->getColorMap()->addColor(ghostColor);
    uint8_t pupilColorIndex = xpms[0]->getColorMap()->addColor(pupilColor);

    //
    // Swap out the ghost color in the pixmaps for the one we were given.
    xpms[0]->replaceColor(xpms[0]->getColorIndexes()[2], ghostColorIndex);
    xpms[1]->replaceColor(xpms[1]->getColorIndexes()[2], ghostColorIndex);
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
    canvas->set(cx, cy, pupilColorIndex);
    canvas->set(cx + 6, cy, pupilColorIndex);
    canvas->set(cx + 1, cy, pupilColorIndex);
    canvas->set(cx + 7, cy, pupilColorIndex);
    canvas->set(cx, cy + 1, pupilColorIndex);
    canvas->set(cx + 6, cy + 1, pupilColorIndex);
    canvas->set(cx + 1, cy + 1, pupilColorIndex);
    canvas->set(cx + 7, cy + 1, pupilColorIndex);
}

bool Ghost::step() {
    bool ret = Sprite::step();
    drawPupils();
    return ret;
}
