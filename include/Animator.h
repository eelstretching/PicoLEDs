#ifndef ANIMATOR_H
#define ANIMATOR_H

#pragma once

#include <vector>

#include "Animation.h"
#include "StopWatch.h"

class Animator : public Animation {
   protected:
    std::vector<Animation *> animations;

    int pos = 0;

    int fps = 30;

    /// @brief The number of microseconds per frame, at our current frame rate.
    float usPerFrame;

    uint missedFrames = 0;

    uint frameCount = 0;

    StopWatch frameWatch;
    StopWatch showWatch;
    StopWatch stepWatch;

    uint64_t startTime = time_us_64();

   public:
    /// @brief Creates an animator for the given canvas.
    /// @param canvas
    Animator(Canvas *canvas, ColorMap *colorMap);
    /// @brief Creates an animator for the given canvas that will run at the
    /// given number of frames per second.
    /// @param canvas the canvas we're animating on
    /// @param fps the number of frames per second to display.
    Animator(Canvas *canvas, ColorMap *colorMap, int fps);
    /// @brief Adds an animation to the end of our list.
    /// @param a the animation to add.
    void add(Animation *a);
    /// @brief Adds a timed animation to the end of our list.
    /// @param a the animation to add.
    /// @param durationMS the duration to run the animation for, in
    /// milliseconds.
    void addTimed(Animation *a, int durationMS);
    void setFPS(int fps);
    int getFPSNeeded() { return fps; };
    /// @brief Steps through whatever animation that we're currently working on.
    /// @return
    bool step();
    void init();
    uint getMissedFrames() { return missedFrames; };
    /// @brief Gets the average frame animation and display time, in
    /// microseconds.

    float getAverageStepTimeUS() { return stepWatch.getAverageTime(); }
    float getAverageFrameTimeUS() { return frameWatch.getAverageTime(); }
    float getAverageShowTimeUS() { return showWatch.getAverageTime(); }

    uint getFrameCount() { return frameCount; };

    float getUsPerFrame() { return usPerFrame; };

    void printStats();
};

#endif