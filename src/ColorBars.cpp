#include "ColorBars.h"

ColorBars::ColorBars(Canvas *canvas, ColorMap *colorMap, uint8_t barWidth, uint8_t nColors) : Animation(canvas, colorMap), barWidth(barWidth), nColors(nColors) {}

bool ColorBars::step() { 
    for(int i = 0; i < canvas->getWidth(); i++) {
        uint8_t colorIndex = (i / barWidth) % nColors;
        canvas->fillColumn(i, colorIndex);
    }
    return true;

 }
