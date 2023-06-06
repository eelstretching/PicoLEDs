#include "StopWatch.h"

#include "pico/stdlib.h"

void StopWatch::combine(StopWatch other) {
  count += other.count;
  totalTime += other.count;
}

void StopWatch::start() {
    currStart = time_us_64();
}

void StopWatch::finish() {
    totalTime += time_us_64() - currStart;
    count++;
}

float StopWatch::getAverageTime() {
    return (float) totalTime / count;
}
