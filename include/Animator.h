#ifndef ANIMATOR_H
#define ANIMATOR_H

#pragma once

#include <vector>

#include "Animation.h"
#include "StopWatch.h"

/// @brief Steps a list of animations and shows them, one frame at a time.
///
/// Frames are drawn at one fixed rate (40 a second unless you say otherwise),
/// and each frame starts on a whole multiple of the frame time on the
/// Animator's clock: frame n starts at n * getUsPerFrame() microseconds. With
/// the plain clock (time_us_64()) that just means steady pacing. A subclass
/// can override clockUs() with a clock shared between Picos (PicoComm's show
/// time), and then every prop running the same frame rate draws its frames at
/// the same instants.
///
/// Each animation is stepped as often as its getFPS() asks, which can be more
/// or less than once a frame; see Animation::getFPS().
class Animator {
   public:
    static constexpr int kDefaultFPS = 40;
    /// @brief Most steps an animation gets in one frame, however far behind
    /// it is.
    static constexpr int kMaxStepsPerFrame = 4;

   protected:
    Canvas* canvas;
    std::vector<Animation*> animations;

    /// @brief The TimedAnimation wrappers that addTimed made, which we delete.
    /// The animations themselves belong to whoever added them.
    std::vector<Animation*> owned;

    int pos = 0;

    int fps = kDefaultFPS;

    /// @brief The number of microseconds per frame, at our frame rate.
    uint32_t usPerFrame;

    /// @brief The clock time this frame was due: a whole multiple of
    /// usPerFrame.
    uint64_t frameTime = 0;

    /// @brief Microseconds of steps the current animation is owed, for
    /// animations that step at their own rate.
    uint32_t stepOwedUs = 0;

    uint missedFrames = 0;

    uint frameCount = 0;

    /// @brief The number of extra times we've re-sent a frame between steps
    /// to keep dithering going.
    uint refreshCount = 0;

    StopWatch frameWatch;
    StopWatch showWatch;
    StopWatch stepWatch;

    uint64_t startTime = time_us_64();

   public:
    /// @brief Creates an animator for the given canvas.
    /// @param canvas
    Animator(Canvas* canvas) : canvas(canvas) { setFPS(kDefaultFPS); };
    /// @brief Creates an animator for the given canvas that will draw the
    /// given number of frames per second, whatever rates its animations step
    /// at.
    /// @param canvas the canvas we're animating on
    /// @param fps the number of frames per second to display.
    Animator(Canvas* canvas, int fps) : canvas(canvas) { setFPS(fps); };
    virtual ~Animator();
    Animator(const Animator&) = delete;
    Animator& operator=(const Animator&) = delete;
    /// @brief Adds an animation to the end of our list.
    /// @param a the animation to add.
    void add(Animation* a);
    /// @brief Adds a timed animation to the end of our list.
    /// @param a the animation to add.
    /// @param durationMS the duration to run the animation for, in
    /// milliseconds.
    void addTimed(Animation* a, int durationMS);
    /// @brief Sets the frame rate: how many frames a second are drawn and
    /// shown. Animations keep stepping at their own rates.
    void setFPS(int fps);
    int getFPS() { return fps; };

    /// @brief The clock frames are paced by, in microseconds. Override it to
    /// pace frames by a clock shared with other Picos. It has to move at the
    /// same rate as time_us_64() (give or take a fraction of a percent), but it
    /// can be offset from it, and can jump.
    virtual uint64_t clockUs() { return time_us_64(); }

    /// @brief The clock time the current frame was due.
    uint64_t getFrameTime() { return frameTime; }
    /// @brief Steps through whatever animation that we're currently working on.
    /// @return
    virtual bool step();

    //
    // A method that you can override to know when an animation changed at the
    // last step.
    virtual void animationChanged() {};
    virtual void init();
    uint getMissedFrames() { return missedFrames; };
    /// @brief Gets the average frame animation and display time, in
    /// microseconds.

    float getAverageStepTimeUS() { return stepWatch.getAverageTime(); }
    float getAverageFrameTimeUS() { return frameWatch.getAverageTime(); }
    float getAverageShowTimeUS() { return showWatch.getAverageTime(); }
    float getAverageDataPrepTimeUS() {return canvas->getRenderer()->getAverageDataSetupTime();}

    uint getFrameCount() { return frameCount; };

    uint getRefreshCount() { return refreshCount; };

    float getUsPerFrame() { return usPerFrame; };

    void printStats();

   protected:
    /// @brief Starts an animation: gives it the clock time it starts at, inits
    /// it, and arranges for its first step to come on the next frame.
    void startAnimation(Animation* a, uint64_t at);

    /// @brief How many times to step the animation this frame.
    int stepsDue(Animation* a);

    /// @brief Waits for the next frame's start, re-sending the current frame
    /// meanwhile if we're dithering. Counts a missed frame if we're already
    /// late.
    virtual void waitForNextFrame();
};

#endif