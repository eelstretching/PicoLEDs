#ifndef COLORBARS_H
#define COLORBARS_H

#pragma once
#include "Animation.h"
#include "Canvas.h"
#include "pico/stdlib.h"
#include "pico/types.h"

class ColorBars : public Animation {
    uint8_t barWidth;
    uint8_t nColors;

   public:
    ColorBars(Canvas* canvas, ColorMap* colorMap, uint8_t barWidth,
              uint8_t nColors);
    virtual bool step() override;
};
#endif