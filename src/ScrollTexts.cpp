#include "ScrollTexts.h"

ScrollText::ScrollText(Canvas* canvas, Font* font, const char* text, int startx,
                       int starty, const RGB& color, RenderAngle angle)
    : Animation(canvas, nullptr, 20),
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
    if (clear) {
        canvas->clear();
    }
    if (!frameDelay) {
        if (frameWait > 0) {
            frameWait--;
            return true;
        }
    }
    font->render(canvas, text, x, y, color, angle);
    switch (angle) {
        case RENDER_0:
            if (direction == LEFT) {
                x--;
                if (x < -width) {
                    x = startx;
                    frameWait = random8(0, 30);
                }
            } else if (direction == RIGHT) {
                x++;
                if (x >= (int)canvas->getWidth()) {
                    x = startx;
                    frameWait = random8(0, 30);
                }
            }
            break;
        case RENDER_90:
            if (direction == UP) {
                y = y + 1;
                if (y >= (int)(canvas->getHeight() + width)) {
                    y = starty;
                    frameWait = random8(0, 30);
                }
            } else if (direction == DOWN) {
                y = y - 1;
                if (y < -width) {
                    y = starty;
                    frameWait = random8(0, 30);
                }
            }
            break;
    }
    return true;
}

void ScrollTexts::init() {
    for (auto t : texts) {
        t->init();
    }
}

bool ScrollTexts::step() {
    canvas->clear();
    for (auto t : texts) {
        t->step();
    }
    return true;
}
