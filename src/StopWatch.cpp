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
    lastTime = time_us_64() - currStart;
    totalTime += lastTime;
    count++;
}

float StopWatch::getAverageTime() {
    return (float) totalTime / count;
}

uint64_t StopWatch::getLastTime() { return lastTime; }

uint64_t StopWatch::getLastTimeMS() { return lastTime/1000; }
