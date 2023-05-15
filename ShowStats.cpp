#include "ShowStats.h"

#include "pico/stdlib.h"

void ShowStats::combine(ShowStats other) {
  showCount += other.showCount;
  showTime += other.showCount;
}

void ShowStats::start() {
    currStart = time_us_64();
}

void ShowStats::finish() {
    showTime += time_us_64() - currStart;
    showCount++;
}

float ShowStats::getAverageShowTime() {
    return (float) showTime / showCount;
}
