#include "ScrollTexts.h"

ScrollText::ScrollText(Canvas* canvas, Font* font, const char* text, int startx,
                       int starty, uint8_t color, RenderAngle angle)
    : Animation(canvas, nullptr),
      startx(startx),
      starty(starty),
      font(font),
      text(text),
      color(color),
      angle(angle) {
    width = font->getWidth(text);
}

void ScrollText::init() {
    x = startx;
    y = starty;
    frameWait = 0;
}

bool ScrollText::step() {
    if(frameWait > 0) {
        frameWait--;
        return true;
    }
    font->render(text, x, y, color, angle);
    switch (angle) {
        case RENDER_0:
            x++;
            if(x >= (int) canvas->getWidth()) {
                x = startx;
                frameWait = random8(0, 30);
            }
            break;
        case RENDER_90:
            y = y + 1;
            if (y >= (int) (canvas->getHeight() + width)) {
                y = starty;
                frameWait = random8(0, 30);
            }
            break;
    }
    return true;
}

void ScrollTexts::init() {
    canvas->setColorMap(colorMap);
    for(auto t : texts) {
        t->init();
    }
}

bool ScrollTexts::step() { 
    canvas->clear();
    for(auto t : texts) {
        t->step();
    }
    return true;
}
