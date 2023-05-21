#ifndef ANIMATION_H
#define ANIMATION_H

#pragma once

#include "Canvas.h"

/// @brief Animation base class with a virtual function for stepping the
/// animation.
class Animation {
    /// @brief The canvas that we're animating on.
    Canvas *canvas;

    /// @brief A stop watch to count our animation time.
    StopWatch aw;

   public:
    /// @brief Construct an animation that will draw on the given canvas.
    /// @param canvas The canvas we'll draw on.
    Animation(Canvas *canvas);

    /// @brief Takes one step in the animation. Someone in charge of the
    /// animations will call this funciton. The default implementation doesn't
    /// do anything.
    virtual void step(){};
};

#endif