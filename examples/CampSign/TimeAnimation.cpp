#include "TimeAnimation.h"

#include "hardware/rtc.h"
#include "pico/types.h"
#include "pico/util/datetime.h"

void TimeAnimation::render() {
    rtc_get_datetime(&dt);
    sprintf(tb, "%02d/%02d %02d:%02d", dt.month, dt.day, dt.hour, dt.min);
    uint w = font->getWidth(tb);
    canvas->clear();
    font->render(tb, (canvas->getWidth() - w)/2, 4, RGB::White);
}

void TimeAnimation::init() {
    render();
}

bool TimeAnimation::step() {
    return true;
}
