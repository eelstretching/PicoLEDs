#ifndef SHOWSTATS_H
#define SHOWSTATS_H

#include "pico/types.h"

#pragma once

class ShowStats {
   public:
    /// @brief The number of times that show has been called
    uint showCount = 0;
    /// @brief The number of microseoncds spent in the show method.
    uint64_t showTime = 0;

    uint64_t currStart = 0;

    /// @brief Combines another set of stats with this one, so we can average
    /// over a bunch of strips.
    /// @param other the stats to combine withours.
    void combine(ShowStats other);

    /// @brief Tell the stats that a show operation is starting.
    void start();

    /// @brief  Tell the stats that a show operation completed.
    void finish();

    float getAverageShowTime();
};

#endif