#ifndef ROTATINGBARS_H
#define ROTATINGBARS_H

#pragma once
#include "pico/stdlib.h"
#include "pico/types.h"
#include "Animation.h"
#include "Canvas.h"

class RotatingBars : public Animation {
    ColorMap *colorMap;
    uint8_t barWidth;
    uint8_t nColors; 
    bool firstFrame = true;
public:
    RotatingBars(Canvas *canvas, ColorMap *colorMap, uint8_t barWidth, uint8_t nColors);
    int getFPSNeeded() override { return 50; };
    virtual void init() override {firstFrame = true; canvas->setColorMap(colorMap);};
    virtual bool step() override;
};
#endif