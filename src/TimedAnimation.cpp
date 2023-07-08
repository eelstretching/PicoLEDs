#include "TimedAnimation.h"

#include "pico/stdlib.h"

TimedAnimation::TimedAnimation(Animation* animation, uint durationMS) {
    this->canvas = animation->getCanvas();
    //
    // We'll take MS as input, but we'll keep time in US.
    duration = durationMS * 1000;
}

void TimedAnimation::init() {
    start = time_us_64();
}

bool TimedAnimation::step() { 
    animation->step();
    return time_us_64() - start < duration;
}
