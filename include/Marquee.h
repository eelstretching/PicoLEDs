#ifndef MARQUEE_H
#define MARQUEE_H

#pragma once

#include "Animation.h"
#include "Direction.h"

class Marquee : Animation {
   private:
    int nColors;
    RGB *colors;
    Direction direction;
    int width;
    int pos;
    int coord;
    int cc = 0;

   public:
    Marquee(Canvas* canvas, ColorMap* colorMap, int nColors, uint8_t *colors,
            int width, Direction direction, int pos, int coord)
        : Animation(canvas, colorMap),
          nColors(nColors),
          width(width),
          direction(direction),
          pos(pos),
          coord(coord) {
            this->colors = new RGB[nColors];
            for (int i = 0; i < nColors; i++) {
                this->colors[i] = colorMap->getColor(colors[i]);
            }
          }

    bool step() override;
};

#endif