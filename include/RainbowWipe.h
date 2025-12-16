#ifndef RAINBOWWIPE_H
#define RAINBOWWIPE_H

#pragma once

#include "Animation.h"
#include "Direction.h"

class RainbowWipe : public Animation {
    protected:
    int p;
    bool clearing;
    Direction direction;

    public:

    RainbowWipe(Canvas *canvas, ColorMap *colorMap) : Animation(canvas, colorMap, 55) {};
    void init();
    bool step();
};

#endif