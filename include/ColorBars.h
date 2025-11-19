#ifndef COLORBARS_H
#define COLORBARS_H

#pragma once
#include "pico/stdlib.h"
#include "pico/types.h"
#include "Animation.h"
#include "Canvas.h"

class ColorBars : public Animation {
    uint8_t barWidth;
    uint8_t nColors; 
public:
    ColorBars(Canvas *canvas, uint8_t barWidth, uint8_t nColors);
    int getFPSNeeded() override { return 1; };
    virtual bool step() override;
};
#endif