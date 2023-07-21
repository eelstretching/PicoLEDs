#ifndef MIDDLEWIPE_H
#define MIDDLEWIPE_H

#pragma once

#include "Animation.h"
#include "Direction.h"

class RainbowWipe : public Animation {
    protected:
    int p;
    bool clearing;
    Direction direction;

    public:
    RainbowWipe(Canvas *canvas) : Animation(canvas) {};
    void init();
    bool step();
};

#endif