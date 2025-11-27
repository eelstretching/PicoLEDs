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

    /// @brief The color map that we're using for this animation.
    ColorMap *colorMap;

    /// @brief A stop watch to count our animation time.
    StopWatch aw;

   public:
    /// @brief Default constructor
    Animation();
    /// @brief Construct an animation that will draw on the given canvas.
    /// @param canvas The canvas we'll draw on.
    Animation(Canvas *canvas, ColorMap *colorMap);

    /// @brief Gets the frames-per-second this animation requires. Default
    /// is 30.
    /// @return the FPS this animation needs.
    virtual int getFPSNeeded() { return 30; };

    /// @brief Virtual destructor!
    virtual ~Animation(){};

    /// @brief Initializes the animation, possibly after it has run to
    /// completion previously.
    virtual void init(){canvas->setColorMap(colorMap);};

    /// @brief Takes one step in the animation. Someone in charge of the
    /// animations will call this function. The default implementation doesn't
    /// do anything.
    /// @returns true if the animation will continue after this step, false if
    /// the animation is complete.
    virtual bool step() { return true; };

    /// @brief Finishes the animation. Can be used to clear up, etc.
    virtual void finish(){};

    Canvas *getCanvas() { return canvas; };
};

class MultiAnimation : public Animation {
    std::vector<Animation *> animations;

    int na;
    int fps = -1;

   public:
    MultiAnimation(Canvas *canvas, ColorMap *colorMap) : Animation(canvas, colorMap){};
    void add(Animation *animation) {
        animations.push_back(animation);
        fps = MAX(fps, animation->getFPSNeeded());
    }

    bool step() {
        for(auto a : animations) {
            a->step();
        }
        return true;
    }
    int getFPSNeeded() { return fps; }
};

#endif