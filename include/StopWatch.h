#ifndef SHOWSTATS_H
#define SHOWSTATS_H

#include "pico/types.h"

#pragma once

class StopWatch {
   public:
    /// @brief The number of times that the watch has been clicked.
    uint count = 0;

    /// @brief The number of microseconds spent running.
    uint64_t totalTime = 0;

    /// @brief The duration of the last timing.
    uint64_t lastTime = 0;

    uint64_t currStart = 0;

    /// @brief Combines another set of stats with this one, so we can average
    /// over a bunch of strips.
    /// @param other the stats to combine withours.
    void combine(StopWatch other);

    /// @brief Tell the watch that an operation is starting.
    void start();

    /// @brief  Tell the watch that an operation completed.
    void finish();

    /// @brief Gets the average time that the watch has run for.
    /// @return the average time, in microseconds.
    float getAverageTime();

    /// @brief Gets the last time in microseconds.
    /// @return the last time in microseconds.
    uint64_t getLastTime();

    /// @brief Gets the last time in milliseconds.
    float getLastTimeMS();
};

#endif