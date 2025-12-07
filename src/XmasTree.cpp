#include "XmasTree.h"

int sideOfLine(int x, int y, int offset) { return x - y + offset; }
bool inside(uint8_t x, uint8_t y, uint8_t llx, uint8_t lly, uint8_t urx,
            uint8_t ury) {
    return x >= llx && x <= urx && y >= lly && y <= ury;
}

Ornament::Ornament(Canvas* canvas, Xpm* orn)
    : Animation(canvas, nullptr), orn(orn) {}

void Ornament::setXY(uint8_t x, uint8_t y) {
    this->x = x;
    this->y = y;
}

void Ornament::setColor(uint8_t color) { cmap[0] = color; }

bool Ornament::impinges(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    uint8_t urx = x + orn->getWidth();
    uint8_t ury = y + orn->getHeight();
    return inside(x1, y1, x, y, urx, ury) || inside(x2, y2, x, y, urx, ury);
}

void Ornament::init() {}

bool Ornament::step() {
    orn->render(canvas, cmap, x, y);
    return true;
}

void Ribbon::init() {
    state = RibbonState::DRAWING;
    lineStarts.clear();
    x = 0;
    y = 0;
}

bool Ribbon::impinges(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    for (auto sx : lineStarts) {
        int lls = sideOfLine(x1, y1, sx);
        int urs = sideOfLine(x2, y2, sx);
        //
        // If the lower left and upper right are on opposite sides of the line,
        // it impinges.
        if ((lls <= 0) && (urs >= 0)) {
            return true;
        }
    }
    return false;
}

bool Ribbon::step() {
    //
    // Remember where we started when we're at zero. Put the offsets of
    // both sides of the ribbon.
    if (y == 0) {
        lineStarts.push_back(x);
        lineStarts.push_back(x + width);
    }
    //
    // The ribbon will wrap itself around the tree as it animates, one row per
    // frame.
    if (state == RibbonState::DRAWING) {
        for (int i = 0; i < width; i++) {
            canvas->set(x + i, y, color);
        }
        y++;
        //
        // Wrap around as though it were wrapped on a cone, but not quite that
        // much, since our tree's a little more than 180 degrees.
        if (y == canvas->getHeight()) {
            y = 0;
            x = x + (canvas->getHeight() / 2);
        } else {
            x++;
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
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            canvas->set(x + i, y + j, colors[ci]);
        }
    }
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
    ribbon = new Ribbon(canvas, 0, 5);
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
    uint8_t randx = 0;
    uint8_t urandx = 0;
    uint8_t randy = 0;
    uint8_t urandy = 0;

    //
    // Find positions for the ornaments that don't impinge on the ribbon or other ornaments.
    for (int i = 0; i < nOrnaments; i++) {
        bool imp = true;
        //
        // Find ornament positions that don't impinge on the ribbon.
        while (imp) {
            randx = random8(0, canvas->getWidth() - ornament->getWidth());
            randy = random8(0, canvas->getHeight() - ornament->getHeight());
            urandx = randx + ornaments[i]->orn->getWidth();
            urandy = randy + ornaments[i]->orn->getHeight();
            if (ribbon->impinges(randx, randy, urandx, urandy)) {
                continue;
            }
            bool foundImpingement = false;
            for(int j = 0; j < i; j++) {
                if (ornaments[j]->impinges(randx, randy, urandx, urandy)) {
                    foundImpingement = true;
                    break;
                }
            }
            if(!foundImpingement) {
                imp = false;
            }
            ornaments[i]->setXY(randx, randy);
            ornaments[i]->setColor(colorMap->getRandomColor());
        }
    }

    //
    // Lights don't want to impinge on teh ribbon or ornaments.
    for (int i = 0; i < nLights; i++) {
        bool imp = true;
        while (imp) {
            randx = random8(0, canvas->getWidth() - lights[i]->size);
            randy = random8(0, canvas->getHeight() - lights[i]->size);
            if (ribbon->impinges(randx, randy, randx + lights[i]->size,
                                 randy + lights[i]->size)) {
                continue;
            }
            bool foundImpingement = false;
            for (int j = 0; j < nOrnaments; j++) {
                if (ornaments[j]->impinges(randx, randy,
                                           randx + lights[i]->size,
                                           randy + lights[i]->size)) {
                    foundImpingement = true;
                    break;
                }
            }
            if (!foundImpingement) {
                imp = false;
            }
        }
        lights[i]->setXY(randx, randy);
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
