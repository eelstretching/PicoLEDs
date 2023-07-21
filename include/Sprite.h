#ifndef SPRITE_H
#define SPRITE_H

#pragma once

#include <vector>

#include "Animation.h"
#include "Direction.h"
#include "Canvas.h"
#include "Xpm.h"

class Sprite : public Animation {
   protected:
    std::vector<Xpm *> frames;
    int startX;
    int startY;
    int pos;

    int x;
    int y;
    Direction direction;

    /// @brief A color map for rendering.
    RGB *colorMap;

   public:
    Sprite(Canvas *canvas, int startX, int startY)
        : Animation(canvas), startX(startX), startY(startY){colorMap = NULL; };
    void add(Xpm *frame);
    Sprite(Sprite &o);
    std::vector<Xpm *> &getFrames() { return frames; };
    void setDirection(Direction direction) {
        this->direction = direction;
    };
    Direction getDirection() {return direction;};
    
    void setStartPosition(int startX, int startY) {
        this->startX = startX;
        this->startY = startY;
    }
    uint getWidth();
    uint getHeight();

    int getX() {return x;};
    int getY() {return y;};

    void init();
    bool step();
};

#endif
