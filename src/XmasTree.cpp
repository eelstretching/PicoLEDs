#include "XmasTree.h"

Ornament::Ornament(Canvas* canvas, Xpm* orn)
    : Animation(canvas, nullptr), orn(orn) {}

void Ornament::setXY(uint8_t x, uint8_t y) {
    this->x = x;
    this->y = y;
}

void Ornament::setColor(uint8_t color) { cmap[0] = color; }

void Ornament::init() {}

bool Ornament::step() {
    orn->render(canvas, cmap, x, y);
    return true;
}

void Ribbon::init() {
    state = RibbonState::DRAWING;
    x = startx;
    y = starty;
}

bool Ribbon::step() {
    //
    // The ribbon will wrap itself around the tree as it animates, one row per
    // frame.
    if (state == RibbonState::DRAWING) {
        for (int i = 0; i < width; i++) {
            canvas->set(x + i, y, color);
            y++;
            //
            // Wrap around as though it were wrapped on a cone.
            if (y == canvas->getHeight()) {
                y = 0;
                x = x + canvas->getHeight();
            } else {
                x++;
            }
        }
        if (x >= canvas->getWidth()) {
            state = RibbonState::DONE;
        }
    }
    return true;
}

void Light::setXY(uint8_t x, uint8_t y) {
    this->x = x;
    this->y = y;
}

void Light::init() {
    currFrame = 0;
    ci = 0;
}

bool Light::step() {
    canvas->set(x, y, colors[ci]);
    currFrame++;
    if (currFrame == steadyFrames) {
        ci = (ci + 1) % nColors;
        currFrame = 0;
    }
    return true;
}

XmasTree::XmasTree(Canvas* canvas, ColorMap* colorMap, uint8_t nOrnaments,
                   uint8_t nLights)
    : Animation(canvas, colorMap), nOrnaments(nOrnaments), nLights(nLights) {
    ribbon = new Ribbon(canvas, 0, 0, 0, 5);
    ornament = new Xpm(ornaPixMap);
    ornaments = new Ornament*[nOrnaments];
    for (int i = 0; i < nOrnaments; i++) {
        ornaments[i] = new Ornament(canvas, ornament);
    }
    lights = new Light*[nLights];
    uint8_t lightColors[3];
    for (int i = 0; i < 3; i++) {
        lightColors[i] = colorMap->getRandomColor();
    }
    for (int i = 0; i < nLights; i++) {
        lights[i] = new Light(canvas, lightColors, 3, 30 + random8(30));
    }
}

void XmasTree::init() {
    state = TreeState::RIBBON;
    canvas->setColorMap(colorMap);
    canvas->clear();
    ribbon->init();
    for (int i = 0; i < nOrnaments; i++) {
        ornaments[i]->setXY(random8(0, canvas->getWidth()),
                            random8(0, canvas->getHeight()));
        ornaments[i]->setColor(colorMap->getRandomColor());
    }
    for (int i = 0; i < nLights; i++) {
        lights[i]->setXY(random8(0, canvas->getWidth()),
                         random8(0, canvas->getHeight()));
    }
}

bool XmasTree::step() {
    switch (state) {
        case RIBBON:
            ribbon->step();
            if (ribbon->state == RibbonState::DONE) {
                state = TreeState::ORNAMENTS;
                el = 0;
                showDelay = 30;
            }
            break;
        case ORNAMENTS:
            showDelay--;
            if (showDelay == 0) {
                ornaments[el++]->step();
                showDelay = 30;
                if (el >= nOrnaments) {
                    el = 0;
                    state = TreeState::LIGHTS;
                }
            }
            break;
        case LIGHTS:
            showDelay--;
            if (showDelay == 0) {
                lights[el++]->step();
                showDelay = 30;
                if (el >= nLights) {
                    el = 0;
                    state = TreeState::BLINKING;
                }
            }
            break;
        case BLINKING:
            for (int i = 0; i < nLights; i++) {
                lights[i]->step();
            }
            break;
    }
    return true;
}
