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

    //
    // Take a step.
    x += deltaX;
    y += deltaY;

    bool someRendered = frames[pos]->render(canvas, myColors, x, y);
    pos = (pos + 1) % frames.size();

    //
    // clear out the pixels left behind
    for(int i = 0; i < deltaX; i++) {
        for(int j = 0; j < h; j++) {
            canvas->set(prevX + i, prevY + j, canvas->getBackground());
        }
    }
    for(int i = 0; i < deltaY; i++) {
        for(int j = 0; j < w; j++) {
            canvas->set(prevX + j, prevY + i, canvas->getBackground());
        }
    }
    return someRendered;
}
