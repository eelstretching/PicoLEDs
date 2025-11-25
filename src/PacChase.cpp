#include "PacChase.h"

#include "Ghost.h"
#include "PacMan.h"
#include "math8.h"

#define PILL_POSITION 61

PacChase::PacChase(Canvas *canvas, ColorMap *colorMap) : Animation(canvas, colorMap) {
    //
    // Make a color map for this animation.
    colorMap = canvas->makeColorMap(32);
    pacMan = new PacMan(canvas, colorMap, 0, 1);
    ghosts = new Sprite *[4];
    ghosts[0] = new Ghost(canvas, colorMap, inkyColor, 0, 1);
    ghosts[1] = new Ghost(canvas, colorMap, blinkyColor, 0, 1);
    ghosts[2] = new Ghost(canvas, colorMap, pinkyColor, 0, 1);
    ghosts[3] = new Ghost(canvas, colorMap, clydeColor, 0, 1);
    dotColorIndex = canvas->getColorMap()->addColor(dotColor);
    setup();
}

/// @brief A constructor that lets us borrow the bitmaps from a pac-man wipe.
/// @param wipe The wipe we'll borrow from.
PacChase::PacChase(PacWipe *wipe, ColorMap *colorMap) : Animation(wipe->canvas, colorMap) {
    canvas = wipe->canvas;
    pacMan = wipe->sprites[0];
    ghosts = &(wipe->sprites[1]);
    setup();
}

void PacChase::setup() {
    pmw = pacMan->getWidth();
    ghw = ghosts[0]->getWidth();
    pilled = new Sprite *[2];
    pilled[0] = new Sprite(canvas, colorMap, 0, 0);
    Xpm *pil1 = new Xpm(pilled1, colorMap);
    pilled[0]->add(pil1);
    pilled[0]->add(pil1);
    pilled[0]->add(pil1);
    pilled[0]->add(pil1);
    Xpm *pil2 = new Xpm(pilled2, colorMap);
    pilled[0]->add(pil2);
    pilled[0]->add(pil2);
    pilled[0]->add(pil2);
    pilled[0]->add(pil2);
    pilled[1] = new Sprite(*pilled[0]);
    piw = pilled[0]->getWidth();
    state = PLAIN;
    pill = new Xpm(power, colorMap);
}

void PacChase::drawDot(int i) {
    canvas->set(i, 7, dotColorIndex);
    canvas->set(i + 1, 7, dotColorIndex);
    canvas->set(i, 8, dotColorIndex);
    canvas->set(i + 1, 8, dotColorIndex);
}

void PacChase::init() {

    //
    // Install our color map and clear the canvas.
    canvas->setColorMap(colorMap);
    canvas->clear();

    //
    // Choose two random ghosts for this.
    g1 = ghosts[random8(4)];
    g2 = g1;
    while (g2 == g1) {
        g2 = ghosts[random8(4)];
    }

    //
    // The power pill!
    pill->render(canvas, PILL_POSITION, 4);

    //
    // Set up for initial animation.
    state = PLAIN;
    pacMan->setStartPosition(-pmw, 1);
    pacMan->setDirection(RIGHT);
    pacMan->init();
    g1->setStartPosition(canvas->getWidth(), 1);
    g1->setDirection(LEFT);
    g1->init();
    g2->setStartPosition(canvas->getWidth() + 6 + ghw, 1);
    g2->setDirection(LEFT);
    g2->init();

    for (int i = 0; i < canvas->getWidth(); i += 8) {
        drawDot(i);
    }
}

bool PacChase::step() {
    pacMan->step();
    g1->step();
    g2->step();
    if (state == PLAIN) {
        //
        // When pacman gets the pill, the ghosts change!
        if (pacMan->getX() + pmw >= PILL_POSITION) {
            state = POWER_PILL;
            pilled[0]->setStartPosition(g1->getX(), g1->getY());
            pilled[0]->setDirection(RIGHT);
            pilled[1]->setStartPosition(g2->getX(), g2->getY());
            pilled[1]->setDirection(RIGHT);
            g1 = pilled[0];
            g2 = pilled[1];
            g1->init();
            g2->init();
        }

        //
        // Re-draw the dots the ghosts nuked.
        for (int i = g1->getX() + ghw; i < g2->getX(); i++) {
            if (i % 8 == 0) {
                drawDot(i);
                break;
            }
        }
        for (int i = g2->getX() + ghw; i < canvas->getWidth(); i++) {
            if (i % 8 == 0) {
                drawDot(i);
                break;
            }
        }
    } else {
        //
        // The animation is over when pacman leaves the screen.
        if (pacMan->getX() >= canvas->getWidth()) {
            return false;
        }
        for (int i = g2->getX(); i >= g1->getX() + ghw; i--) {
            if (i % 8 == 0) {
                drawDot(i);
            }
        }
        for (int i = g1->getX(); i >= pacMan->getX() + pmw; i--) {
            if (i % 8 == 0) {
                drawDot(i);
            }
        }
    }
    return true;
}
