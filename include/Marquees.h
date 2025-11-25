#ifndef MARQUEES_H
#define MARQUEES_H

#pragma once
#include "Animation.h"
#include "Direction.h"
#include "Marquee.h"

class Marquees : public Animation {
   public:
    Marquees(Canvas *canvas, ColorMap *colorMap, int nColors, int width, Direction direction, int nMarquees = -1);

    ~Marquees();

    bool step() override;

    int getFPSNeeded() override { return 60; };

   private:
    RGB *colors;
    Direction direction;
    int width;
    Marquee **marquees;
    int nMarquees;
 };

#endif