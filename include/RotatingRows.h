#ifndef ROTATINGROWS_H
#define ROTATINGROWS_H

#pragma once
#include "pico/stdlib.h"
#include "pico/types.h"
#include "Animation.h"
#include "Canvas.h"

class RotatingRows : public Animation {
    uint8_t rowWidth;
    uint8_t nColors; 
    uint8_t *colors;
    bool firstFrame = true;
public:
    RotatingRows(Canvas *canvas, ColorMap *colorMap, uint8_t nColors, uint8_t *colors, uint8_t rowWidth);
    virtual void init() override {firstFrame = true; canvas->setColorMap(colorMap);};
    virtual bool step() override;
};
#endif