#ifndef FADECOLORMAP_H
#define FADECOLORMAP_H
#pragma once

#include "ArrayColorMap.h"

/// @brief A color map that provides faded versions of colors in an underlying
/// ArrayColorMap.
class FadeColorMap : public ArrayColorMap {
   protected:
    uint8_t nSteps;

   public:
    FadeColorMap(ColorMap* baseMap, uint8_t nColors, uint8_t nSteps, uint8_t fadeFactor = 40);
    ~FadeColorMap();
    
};

#endif