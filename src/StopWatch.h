#ifndef SHOWSTATS_H
#define SHOWSTATS_H

#include "pico/types.h"

#pragma once

class StopWatch {
   public:
    /// @brief The number of times that show has been called
    uint count = 0;
    /// @brief The number of microseoncds spent in the show method.
    uint64_t totalTime = 0;

    uint64_t currStart = 0;

    /// @brief Combines another set of stats with this one, so we can average
    /// over a bunch of strips.
    /// @param other the stats to combine withours.
    void combine(StopWatch other);

    /// @brief Tell the stats that a show operation is starting.
    void start();

    /// @brief  Tell the stats that a show operation completed.
    void finish();

    float getAverageTime();
};

#endif