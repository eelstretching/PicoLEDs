#ifndef ROTATINGCOLUMNS_H
#define ROTATINGCOLUMNS_H

#pragma once
#include "pico/stdlib.h"
#include "pico/types.h"
#include "Animation.h"
#include "Canvas.h"

class RotatingColumns : public Animation {
    uint8_t colWidth;
    uint8_t nColors; 
    bool firstFrame = true;
public:
    RotatingColumns(Canvas *canvas, ColorMap *colorMap, uint8_t colWidth, uint8_t nColors);
    int getFPSNeeded() override { return 50; };
    virtual void init() override {firstFrame = true; canvas->setColorMap(colorMap);};
    virtual bool step() override;
};
#endif