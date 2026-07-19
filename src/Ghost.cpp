#include "Ghost.h"
#include "Ghost.h"

Ghost::Ghost(Canvas* canvas, Xpm** frames, const RGB& ghostColor,
             const RGB& pupilColor, int startX, int startY, Direction dir)
    : Sprite(canvas, startX, startY, dir),
      frames(frames),
      ghostColor(ghostColor),
      pupilColor(pupilColor) {

    myColors = new RGB[frames[0]->getNumberOfColors()];
    memcpy(myColors, frames[0]->getColors(),
           frames[0]->getNumberOfColors() * sizeof(RGB));
    myColors[1] = ghostColor;
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
}

Ghost::~Ghost() {}

//
// We're going to assume that we're called after the animation step, at which
// point the pixels have been rendered into the canvas and now we need to add
// the pupil in the right place.
void Ghost::drawPupils() {
    //
    // figure out the bottom-left coordinate of the eye on the canvas.
    uint cx, cy;
    uint h = frames[0]->getHeight();

    if (deltaY > 0) {
        cx = x + 2;
        cy = y + 9;
    } else if (deltaY < 0) {
        cx = x + 2;
        cy = y + 6;
    } else if (deltaX < 0) {
        cx = x + 1;
        cy = y + 8;
    } else if (deltaX > 0) {
        cx = x + 4;
        cy = y + 8;
    }

    //
    // Put the four pupil-colored pixels into each eye.
    canvas->set(cx, cy, pupilColor);
    canvas->set(cx + 6, cy, pupilColor);
    canvas->set(cx + 1, cy, pupilColor);
    canvas->set(cx + 7, cy, pupilColor);

    canvas->set(cx, cy + 1, pupilColor);
    canvas->set(cx + 6, cy + 1, pupilColor);
    canvas->set(cx + 1, cy + 1, pupilColor);
    canvas->set(cx + 7, cy + 1, pupilColor);
}

bool Ghost::step() {
    bool ret = Sprite::step();
    drawPupils();
    return ret;
}

void Ghost::init() {
    x = startX;
    y = startY;
}