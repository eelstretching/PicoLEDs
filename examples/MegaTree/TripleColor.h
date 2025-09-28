#ifndef TRIPLECOLOR_H
#define TRIPLECOLOR_H

#pragma once

#include <stdint.h>
#include "Animation.h"
#include "Canvas.h"

/// @brief A light on a tree that fades in and out with three colors.
class TripleColor : public Animation {
    /// @brief the canvas that we'll render our colors onto.
    Canvas *canvas;

    RGB *colors;

    int currRow = 0;

    public:
    /// @brief Construct a TripleColor animation.
    /// @param canvas The canvas to draw on.
    /// @param colors An array of three RGB colors to use for the animation.
    TripleColor(Canvas *canvas);

    /// @brief Default destructor.
    ~TripleColor();

    /// @brief Gets the frames-per-second this animation requires. Default
    /// is 30.
    /// @return the FPS this animation needs.
    int getFPSNeeded() override { return 8; };
    
    /// @brief Takes one step in the animation.
    /// @returns true if the animation will continue after this step, false if
    /// the animation is complete.
    bool step() override;
};
#endif