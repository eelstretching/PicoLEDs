#ifndef FIRE_H
#define FIRE_H

#pragma once

#include "Animation.h"

class Fire : Animation {
    /// @brief the canvas that we'll render our fire onto.
    Canvas *canvas;

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

   public:
    Fire(Canvas *canvas, uint row)
        : Fire(canvas, row, 0, canvas->getWidth(), 55, 120){};

    Fire(Canvas *canvas, uint row, uint x, uint n, uint cooling, uint sparking);

    ~Fire();

    void step();
};
#endif