#include "PacChase.h"

#include "Ghost.h"
#include "PacMan.h"
#include "math8.h"

#define PILL_POSITION 61

PacChase::PacChase(Canvas *canvas) : Animation(canvas) {
    pacMan = new PacMan(canvas, 0, 1);
    ghosts = new Sprite *[4];
    ghosts[0] = new Ghost(canvas, inkyColor, 0, 1);
    ghosts[1] = new Ghost(*((Ghost *)ghosts[0]), blinkyColor, 0, 1);
    ghosts[2] = new Ghost(*((Ghost *)ghosts[0]), pinkyColor, 0, 1);
    ghosts[3] = new Ghost(*((Ghost *)ghosts[0]), clydeColor, 0, 1);
    setup();
}

/// @brief A constructor that lets us borrow the bitmaps from a pac-man wipe.
/// @param wipe The wipe we'll borrow from.
PacChase::PacChase(PacWipe *wipe) {
    canvas = wipe->canvas;
    pacMan = wipe->sprites[0];
    ghosts = &(wipe->sprites[1]);
    setup();
}

void PacChase::setup() {
    pmw = pacMan->getWidth();
    ghw = ghosts[0]->getWidth();
    pilled = new Sprite *[2];
    pilled[0] = new Sprite(canvas, 0, 0);
    Xpm *pil1 = new Xpm(pilled1);
    pilled[0]->add(pil1);
    pilled[0]->add(pil1);
    pilled[0]->add(pil1);
    pilled[0]->add(pil1);
    Xpm *pil2 = new Xpm(pilled2);
    pilled[0]->add(pil2);
    pilled[0]->add(pil2);
    pilled[0]->add(pil2);
    pilled[0]->add(pil2);
    pilled[1] = new Sprite(*pilled[0]);
    piw = pilled[0]->getWidth();
    state = PLAIN;
    pill = new Xpm(power);
}

void PacChase::drawDot(int i) {
    canvas->set(i, 7, dotColor);
    canvas->set(i + 1, 7, dotColor);
    canvas->set(i, 8, dotColor);
    canvas->set(i + 1, 8, dotColor);
}

void PacChase::init() {
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
    pacMan->setDirection(SpriteDirection::SPRITE_RIGHT);
    pacMan->init();
    g1->setStartPosition(canvas->getWidth(), 1);
    g1->setDirection(SpriteDirection::SPRITE_LEFT);
    g1->init();
    g2->setStartPosition(canvas->getWidth() + 6 + ghw, 1);
    g2->setDirection(SpriteDirection::SPRITE_LEFT);
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
            pilled[0]->setDirection(SpriteDirection::SPRITE_RIGHT);
            pilled[1]->setStartPosition(g2->getX(), g2->getY());
            pilled[1]->setDirection(SpriteDirection::SPRITE_RIGHT);
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
