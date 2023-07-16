#ifndef SPRITE_H
#define SPRITE_H

#pragma once

#include <vector>

#include "Animation.h"
#include "Canvas.h"
#include "Xpm.h"

enum SpriteDirection {
    UP,
    DOWN,
    LEFT,
    RIGHT,
};

class Sprite : public Animation {
   protected:
    std::vector<Xpm *> frames;
    int startX;
    int startY;
    int pos;

    int x;
    int y;
    SpriteDirection direction;

    /// @brief A color map for rendering.
    RGB *colorMap;

   public:
    Sprite(Canvas *canvas, int startX, int startY)
        : Animation(canvas), startX(startX), startY(startY){colorMap = NULL;};
    void add(Xpm *frame);
    std::vector<Xpm *> &getFrames() { return frames; };
    void setDirection(SpriteDirection direction) {
        this->direction = direction;
    };
    void init();
    bool step();
};

#endif