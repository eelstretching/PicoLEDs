#ifndef ROTRANDROWS_H
#define ROTRANDROWS_H

#pragma once

#include <stdint.h>
#include "Animation.h"
#include "Canvas.h"
#include "pico/stdlib.h"
#include "pico/types.h"

class RotRandRows : public Animation {
    uint8_t rowWidth;
    uint8_t nColors;
    uint8_t currColors[8];
    bool firstFrame = true;

   public:
    RotRandRows(Canvas* canvas, ColorMap* colorMap, uint8_t nColors);
    virtual void init() override;
    virtual bool step() override;
};
#endif