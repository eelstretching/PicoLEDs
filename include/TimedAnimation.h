#ifndef TIMEDANIMATION_H
#define TIMEDANIMATION_H

#pragma once

#include "pico/types.h"

#include "Animation.h"

/// @brief A class that wraps a never-ending animation and makes it time-limited.
class TimedAnimation : public Animation {
    protected:
    /// @brief Our wrapped never-ending animation.
    Animation *animation;
    /// @brief How long the animation should run for, in microseconds.
    uint duration;

    /// @brief When this round of the animation started, in microseconds.
    uint64_t start;

    public:
    /// @brief Creates a time-limited animation from a never ending one.
    /// @param animation the never-ending animation.
    /// @param durationMS How many milliseconds we should let the animation run for.
    TimedAnimation(Animation *animation, uint durationMS);
    /// @brief Resets our timing, so we can re-run for the given duration.
    void init();
    /// @brief Steps the underlying animation.
    /// @return true if the animation is not out of time, false if we're out of time.
    bool step();
    int getFPSNeeded() {return animation->getFPSNeeded();};

};

#endif