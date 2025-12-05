#include "Sprite.h"

void Sprite::add(Xpm* frame) { frames.push_back(frame); }

Sprite::Sprite(Sprite& o) {
    canvas = o.canvas;
    frames = o.frames;
    startX = o.startX;
    startY = o.startY;
    colorMap = NULL;
    init();
}

uint Sprite::getWidth() { 
    uint max = 0;
    for(auto f : frames) {
        if(f->getWidth() > max) {
            max = f->getWidth();
        }
    }
    return max;
}

uint Sprite::getHeight() { 
    uint max = 0;
    for(auto f : frames) {
        if(f->getHeight() > max) {
            max = f->getHeight();
        }
    }
    return max;
}

void Sprite::init() {
    x = startX;
    y = startY;
    pos = 0;
}

bool Sprite::step() {

    //
    // Remember where we were and how big we were, so we can clean up behind ourselves.
    int prevX = x;
    int prevY = y;
    int h = frames[pos]->getHeight();
    int w = frames[pos]->getWidth();
    switch (direction) {
        case UP:
            y++;
            break;
        case DOWN:
            y--;
            break;
        case LEFT:
            x--;
            break;
        case RIGHT:
            x++;
            break;

        default:
            break;
    }
    bool someRendered = frames[pos]->render(canvas, nullptr, x, y);
    pos = (pos + 1) % frames.size();

    //
    // clear out the pixels left behind
    switch (direction) {
        case UP:
            for(int a = x; a < x + w; a++) {
                canvas->set(a, prevY, canvas->getBackgroundIndex());
            }
            break;
        case DOWN:
            for(int a = x; a < x + w; a++) {
                canvas->set(a, prevY+h-1, canvas->getBackgroundIndex());
            }
            break;
        case LEFT:
            for(int a = y; a < y+h-1; a++) {
                canvas->set(prevX+w-1, a, canvas->getBackgroundIndex());
            }
            break;
        case RIGHT:
            for(int a = y; a < y+h-1; a++) {
                canvas->set(prevX, a, canvas->getBackgroundIndex());
            }
            break;
    }
    return someRendered;
}
