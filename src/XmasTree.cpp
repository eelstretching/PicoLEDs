#include "XmasTree.h"

#include "ArrayColorMap.h"

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

void Ornament::init() {
    cmap[0] = canvas->getColorMap()->getRandomColor();
}

bool Ornament::step() {
    orn->render(canvas, cmap, x, y);
    return true;
}

void Ribbon::init() {
    state = RibbonState::DRAWING;
    lineStarts.clear();
    //
    // Figure out the ribbon line start positions.
    uint8_t sx = 0;
    bool done = false;
    while(!done) {
        lineStarts.push_back(sx);
        lineStarts.push_back(sx + width);
        sx += (canvas->getHeight() + canvas->getHeight() / 2);
        if(sx >= canvas->getWidth()) {
            done = true;
        }
    }
    x = 0;
    y = 0;
}

bool Ribbon::step() {
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

XmasTree::XmasTree(Canvas* canvas, uint8_t nOrnaments, uint8_t nLights)
    : Animation(canvas, nullptr), nOrnaments(nOrnaments), nLights(nLights) {
    //
    // We have a pretty specific color map, so just set it here.
    colorMap = new ArrayColorMap({RGB::Red, RGB::Green, RGB::Blue, RGB::White,
                                  RGB(213, 181, 52),  // Gold
                                  RGB::Purple, RGB::Orange, RGB::Silver});
    colorMap->setBackground(RGB::ForestGreen);
    colorMap->setBrightness(16);

    ribbon = new Ribbon(canvas, 0, 5);
    ornament = new Xpm(ornaPixMap);
    nOrnaments = 9;
    ornaments = new Ornament*[9];
    for (int i = 0; i < 9; i++) {
        ornaments[i] = new Ornament(canvas, ornament);
    }
    ornaments[0]->setXY(17, 7);
    ornaments[1]->setXY(18, 15);
    ornaments[2]->setXY(16, 19);
    ornaments[3]->setXY(46, 6);
    ornaments[4]->setXY(46, 14);
    ornaments[5]->setXY(46, 18);
    ornaments[6]->setXY(77, 7);
    ornaments[7]->setXY(77, 15);
    ornaments[8]->setXY(77, 19);
    
    nLights = 18;
    lights = new Light*[nLights];
    uint8_t lightColors[3];
    for (int i = 0; i < 3; i++) {
        lightColors[i] = colorMap->getRandomColor();
    }
    for (int i = 0; i < nLights; i++) {
        lights[i] = new Light(canvas, lightColors, 3, 30 + random8(30));
    }
    lights[0]->setXY(10, 5);
    lights[1]->setXY(25, 10);
    lights[2]->setXY(40, 5);
    lights[3]->setXY(55, 10);
    lights[4]->setXY(70, 5);
    lights[5]->setXY(85, 10);
    lights[6]->setXY(20, 15);
    lights[7]->setXY(35, 20);
    lights[8]->setXY(50, 15);
    lights[9]->setXY(65, 20);
    lights[10]->setXY(80, 15);
    lights[11]->setXY(30, 25);
    lights[12]->setXY(45, 30);
    lights[13]->setXY(60, 25);
    lights[14]->setXY(75, 30);
    lights[15]->setXY(40, 35);
    lights[16]->setXY(55, 40);
    lights[17]->setXY(70, 35);
}

void XmasTree::init() {
    state = TreeState::RIBBON;
    canvas->setColorMap(colorMap);
    canvas->clear();
    ribbon->init();

    for(int i = 0; i < nOrnaments; i++) {
        ornaments[i]->init();
    }

    for(int i = 0; i < nLights; i++) {
        lights[i]->init();
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
