#ifndef FIRE_H
#define FIRE_H

#pragma once

#include "Animation.h"
#include "colorutils.h"

class Fire : public Animation {
    /// @brief The row that we'll be animating
    uint row;

    /// @brief The point in the row where we'll start the fire.
    uint x;

    /// @brief How many LEDs from the origin point will the fire burn?
    uint n;

    /// @brief The point where the burning ends.
    uint end;

    /// @brief An array of 8 bit heats, representing the state of the
    /// simulation.
    uint8_t *heat;

    /// @brief How much does the air cool as it rises?
    // Less cooling = taller flames.  More cooling = shorter flames.
    // Default 50, suggested range 20-100
    uint cooling;
    /// @brief  What chance (out of 255) is there that a new spark will be lit?
    // Higher chance = more roaring fire.  Lower chance = more flickery fire.
    // Default 120, suggested range 50-200.
    uint sparking;

    /// @brief The palette to color with, if usePalette is set.
    RGBPalette16 palette;

    bool usePalette = false;

   public:
    /// @brief A fire that burns along a whole row of the canvas.
    Fire(Canvas *canvas, ColorMap *colorMap, uint row)
        : Fire(canvas, colorMap, 0, canvas->getWidth(), row, 55, 120){};

    /// @brief A fire that burns along part of a row.
    /// @param x where along the row the fire starts
    /// @param n how many pixels long the fire is
    /// @param row the row to burn in
    /// @param cooling how much the air cools as it rises (default 55)
    /// @param sparking the chance out of 255 of a new spark (default 120)
    Fire(Canvas *canvas, ColorMap *colorMap, uint x, uint n, uint row, uint cooling, uint sparking);

    /// @brief Color the fire with a palette instead of the default black-body
    /// heat colors. The coolest heat maps to the start of the palette and the
    /// hottest to near the end, as in FastLED's Fire2012WithPalette.
    void setPalette(const RGBPalette16 &palette) {
        this->palette = palette;
        usePalette = true;
    };

    ~Fire();

    bool step();
};
#endif