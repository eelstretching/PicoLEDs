#ifndef BARBERPOLE_H
#define BARBERPOLE_H

#pragma once

#include "Animation.h"

class BarberPole : public Animation {
    int nColors;

    int stripeWidth;

    int start;

   public:
    /// @brief Create a barber pole animation. The stripes will be four pixels
    /// wide.
    /// @param canvas the canvas we'll draw on
    /// @param c1 the first color for the barber pole
    /// @param c2 the second color for the barber pole
    BarberPole(Canvas *canvas, ColorMap *colorMap, int nColors) : Animation(canvas, colorMap), nColors(nColors) {
        stripeWidth = 4;
    };

    /// @brief Create a barber pole animation.
    /// @param canvas the canvas we'll draw on
    /// @param c1 the first color for the barber pole
    /// @param c2 the second color for the barber pole
    /// @param stripeWidth how wide the stripes should be.
    BarberPole(Canvas *canvas, ColorMap *colorMap, int nColors, int stripeWidth)
        : Animation(canvas, colorMap), nColors(nColors), stripeWidth(stripeWidth) {
    };

    int getFPSNeeded() { return 10; };
    void init();

    bool step();
};

#endif