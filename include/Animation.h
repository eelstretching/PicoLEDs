#ifndef ANIMATION_H
#define ANIMATION_H

#pragma once

#include <string.h>

#include "Canvas.h"

/// @brief Animation base class with a virtual function for stepping the
/// animation.
///
/// Who owns what: an animation frees everything it allocates itself, so it can
/// be made with new when the show needs it and deleted afterwards. Anything you
/// hand it by pointer (the canvas, a color map, a list of color indices, the
/// animations you add() to a MultiAnimation or Animator, ScrollTexts and
/// TextElements) still belongs to you, and has to outlive the animation.
/// Animations that own memory can't be copied.
class Animation {
    friend class Animator;

   protected:
    /// @brief The canvas that we're animating on.
    Canvas *canvas;

    /// @brief The color map that we're using for this animation.
    ColorMap *colorMap;

    /// @brief A stop watch to count our animation time.
    StopWatch aw;

    char name[20];

    /// @brief How many times a second this animation wants step() called, or
    /// 0 for an animation that goes by the clock instead (see elapsedUs()).
    uint8_t fps = 30;

    /// @brief The Animator's clock, in microseconds, when this animation
    /// started, and at its latest step. See elapsedUs() and getFrameUs().
    uint64_t startUs = 0;
    uint64_t nowUs = 0;
    /// @brief Microseconds since the previous step.
    uint32_t frameUs = 0;

   public:
    /// @brief Default constructor
    Animation();
    /// @brief Construct an animation that will draw on the given canvas.
    /// @param canvas The canvas we'll draw on.
    Animation(Canvas *canvas, ColorMap *colorMap);
    Animation(Canvas *canvas, ColorMap *colorMap, uint8_t fps);

    /// @brief Gets how many times a second this animation wants to be
    /// stepped. Default is 30.
    ///
    /// The Animator draws frames at one fixed rate for the whole show, and
    /// steps each animation as often as this asks: a 20 fps animation every
    /// other frame at 40 frames a second, a 60 fps one alternately once and
    /// twice per frame. That keeps older animations, which move a fixed amount
    /// per step, running at the speed they always have.
    ///
    /// New animations should return 0 instead: they're stepped exactly once
    /// per frame and work out how far to move from getFrameUs() and
    /// elapsedUs(), stating their speeds per second, so they look the same at
    /// any frame rate. Starlight is an example.
    /// @return steps per second, or 0 for a clock-driven animation.
    virtual int getFPS() { return fps; };

    virtual void setFPS(uint8_t fps) { this->fps = fps; };

    /// @brief Virtual destructor!
    virtual ~Animation(){};

    virtual void setName(const char* n) {strncpy(name, n, 15);};

    virtual const char *getName() {return name;};

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
    virtual void finish(){};

    /// @brief Called by the Animator just before init(), with its clock.
    /// Animations that wrap others pass it on.
    virtual void startClock(uint64_t now) {
        startUs = now;
        nowUs = now;
        frameUs = 0;
    }

    /// @brief Called by the Animator before each step() with the time of the
    /// frame being drawn. A second step in the same frame sees a frameUs of 0,
    /// and the clock never goes backwards here even if the Animator's does.
    virtual void setFrameTime(uint64_t now) {
        if (now <= nowUs) {
            frameUs = 0;
            return;
        }
        frameUs = (uint32_t)(now - nowUs);
        nowUs = now;
    }

    /// @brief Microseconds since this animation started (its last init()).
    uint64_t elapsedUs() const { return nowUs - startUs; }

    /// @brief Microseconds since the previous step: how far to move this step.
    uint32_t getFrameUs() const { return frameUs; }

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
        fps = MAX(fps, animation->getFPS());
    }

    void init() override {
        for (auto a : animations) {
            a->init();
        }
    }

    void startClock(uint64_t now) override {
        Animation::startClock(now);
        for (auto a : animations) {
            a->startClock(now);
        }
    }

    void setFrameTime(uint64_t now) override {
        Animation::setFrameTime(now);
        for (auto a : animations) {
            a->setFrameTime(now);
        }
    }

    bool step() override{
        for(auto a : animations) {
            a->step();
        }
        return true;
    }
    int getFPS() { return fps; }
};

#endif