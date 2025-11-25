#ifndef MARQUEE_H
#define MARQUEE_H

#pragma once

#include "Animation.h"
#include "Direction.h"

class Marquee : Animation {
   public:
    Marquee(Canvas *canvas, ColorMap *colorMap, int nColors, int width, Direction direction, int pos, int coord)
        : Animation(canvas, colorMap), nColors(nColors), width(width), direction(direction), pos(pos), coord(coord) {
    }

    bool step() override;

   private:
    int nColors;
    Direction direction;
    int width;
    int pos;
    int coord;
    int cc = 0;
};

#endif