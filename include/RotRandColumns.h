#ifndef ROTRANDCOLUMNS_H
#define ROTRANDCOLUMNS_H

#pragma once
#include "pico/stdlib.h"
#include "pico/types.h"
#include "Animation.h"
#include "Canvas.h"

class RotRandColumns : public Animation {
    uint8_t colWidth;
    uint8_t nColors; 
    uint8_t currColors[8];
    bool firstFrame = true;
public:
    RotRandColumns(Canvas *canvas, ColorMap *colorMap, uint8_t nColors);
    virtual void init() override;
    virtual bool step() override;
};
#endif