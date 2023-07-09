#ifndef ANIMATION_H
#define ANIMATION_H

#pragma once

#include "Canvas.h"

/// @brief Animation base class with a virtual function for stepping the
/// animation.
class Animation {
    friend class Animator;

   protected:
    /// @brief The canvas that we're animating on.
    Canvas *canvas;

    /// @brief A stop watch to count our animation time.
    StopWatch aw;

   public:
    /// @brief Default constructor
    Animation();
    /// @brief Construct an animation that will draw on the given canvas.
    /// @param canvas The canvas we'll draw on.
    Animation(Canvas *canvas);

    /// @brief Virtual destructor!
    virtual ~Animation(){};

    /// @brief Initializes the animation, possibly after it has run to
    /// completion previously.
    virtual void init(){};

    /// @brief Takes one step in the animation. Someone in charge of the
    /// animations will call this function. The default implementation doesn't
    /// do anything.
    /// @returns true if the animation will continue after this step, false if
    /// the animation is complete.
    virtual bool step() { return true; };

    /// @brief Finishes the animation. Can be used to clear up, etc.
    virtual void finish() {};

    Canvas *getCanvas() { return canvas; };
};

#endif