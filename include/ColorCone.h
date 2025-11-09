#ifndef COLORCONE_H
#define COLORCONE_H

#pragma once

#include "Animation.h"

class ColorCone : public Animation {
   public:

    ColorCone(Canvas *canvas, ColorMap *colorMap);

    void init() override;
    bool step() override;

   private:
    
    ColorMap *colorMap;
    int pos = 0;
    bool buildDone = false;
    uint8_t currCol = 0;
    uint8_t currColor = 0;
    uint8_t bandSize = 0;
};

#endif