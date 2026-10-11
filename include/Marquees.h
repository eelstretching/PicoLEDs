#ifndef MARQUEES_H
#define MARQUEES_H

#pragma once
#include "Animation.h"
#include "Direction.h"
#include "Marquee.h"

class Marquees : public Animation {
   private:
    uint8_t *colors;
    Direction direction;
    int width;
    Marquee **marquees;
    int nMarquees;

    public:
    Marquees(Canvas *canvas, ColorMap *colorMap, int nColors, uint8_t *colors, int width, Direction direction, int nMarquees = -1);

    ~Marquees();
    Marquees(const Marquees&) = delete;
    Marquees& operator=(const Marquees&) = delete;

    bool step() override;

 };

#endif