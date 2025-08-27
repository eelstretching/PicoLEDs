#include "TimeAnimation.h"

#include "hardware/timer.h"
#include "pico/types.h"
#include "pico/util/datetime.h"
#include "pico/printf.h"

#define S_IN_US 1000000

void TimeAnimation::render() {
    last = time_us_64();
    rtc_get_datetime(&dt);
    sprintf(tb, "%02d/%02d    %02d:%02d:%02d", dt.month, dt.day, dt.hour,
            dt.min, dt.sec);
    canvas->clear();
    font->render(tb, 20, 4, RGB::White);
}

void TimeAnimation::init() {
    start = time_us_64();
    render();
}

bool TimeAnimation::step() {
    uint64_t curr = time_us_64();
    if (curr - last >= S_IN_US) {
        render();
    }
    return curr - start < duration;;
}

