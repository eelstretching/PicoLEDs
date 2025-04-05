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

    StopWatch sw;
    StopWatch aw;

   public:
    /// @brief Creates an animator for the given canvas.
    /// @param canvas
    Animator(Canvas *canvas);
    /// @brief Creates an animator for the given canvas that will run at the
    /// given number of frames per second.
    /// @param canvas the canvas we're animating on
    /// @param fps the number of frames per second to display.
    Animator(Canvas *canvas, int fps);
    /// @brief Adds an animation to the end of our list.
    /// @param a the animation to add.
    void add(Animation *a);
    void setFPS(int fps);
    int getFPSNeeded() { return fps; };
    /// @brief Steps through whatever animation that we're currently working on.
    /// @return
    bool step();
    void init();
    uint getMissedFrames() { return missedFrames; };
    /// @brief Gets the average frame animation and display time, in
    /// microseconds.
    float getAverageFrameTimeUS() { return aw.getAverageTime(); }

    float getAverageShowTimeUS() { return sw.getAverageTime(); }

    uint getFrameCount() { return frameCount; };

    float getUsPerFrame() { return usPerFrame; };
};

#endif