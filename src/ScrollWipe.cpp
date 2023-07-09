#include "ScrollWipe.h"

ScrollWipe::ScrollWipe(Canvas* canvas, ScrollDirection dir) : Animation(canvas) {
    this->dir = dir;
    scrolled = 0;
    switch (dir) {
        case UP:
        case DOWN:
            limit = canvas->getHeight();
            break;
        case RIGHT:
        case LEFT:
            limit = canvas->getWidth();
            break;
    }
}

void ScrollWipe::init() { scrolled = 0; }

bool ScrollWipe::step() {
    switch (dir) {
        case UP:
            canvas->scrollUp();
            break;
        case DOWN:
            canvas->scrollDown();
            break;
        case RIGHT:
            canvas->shiftRight(scrolled, 0, canvas->getWidth() - scrolled,
                               canvas->getHeight(), 1);
            break;
        case LEFT:
            canvas->shiftLeft(1, 0, canvas->getWidth() - scrolled,
                              canvas->getHeight(), 1);
            break;
    }
    scrolled++;
    return scrolled < limit;
}

void ScrollWipe::setExtraFrames(uint extraFrames) {
    this->extraFrames = extraFrames;
    limit += extraFrames;
}
