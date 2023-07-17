#include "ScrollWipe.h"

ScrollWipe::ScrollWipe(Canvas* canvas, ScrollDirection dir) : Animation(canvas) {
    this->dir = dir;
    scrolled = 0;
    switch (dir) {
        case SCROLL_UP:
        case SCROLL_DOWN:
            limit = canvas->getHeight();
            break;
        case SCROLL_RIGHT:
        case SCROLL_LEFT:
            limit = canvas->getWidth();
            break;
    }
}

void ScrollWipe::init() { scrolled = 0; }

bool ScrollWipe::step() {
    switch (dir) {
        case SCROLL_UP:
            canvas->scrollUp();
            break;
        case SCROLL_DOWN:
            canvas->scrollDown();
            break;
        case SCROLL_RIGHT:
            canvas->shiftRight(scrolled, 0, canvas->getWidth() - scrolled,
                               canvas->getHeight(), 1);
            break;
        case SCROLL_LEFT:
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
