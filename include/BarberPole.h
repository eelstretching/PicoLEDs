#ifndef BARBERPOLE_H
#define BARBERPOLE_H

#pragma once

#include "Animation.h"

class BarberPole : public Animation {
    RGB c[2];

    int stripeWidth;

    int start;

   public:
    /// @brief Create a barber pole animation. The stripes will be four pixels
    /// wide.
    /// @param canvas the canvas we'll draw on
    /// @param c1 the first color for the barber pole
    /// @param c2 the second color for the barber pole
    BarberPole(Canvas *canvas, RGB c1, RGB c2) : Animation(canvas) {
        stripeWidth = 4;
        c[0] = c1;
        c[1] = c2;
    };

    /// @brief Create a barber pole animation.
    /// @param canvas the canvas we'll draw on
    /// @param c1 the first color for the barber pole
    /// @param c2 the second color for the barber pole
    /// @param stripeWidth how wide the stripes should be.
    BarberPole(Canvas *canvas, RGB c1, RGB c2, int stripeWidth)
        : Animation(canvas), stripeWidth(stripeWidth) {
        c[0] = c1;
        c[1] = c2;
    };

    int getFPSNeeded() { return 2; };
    void init();

    bool step();
};

#endif