#include "TimeAnimation.h"

#include "pico/types.h"
#include "hardware/rtc.h"
#include "pico/util/datetime.h"


bool TimeAnimation::step() {
    curr = time_us_64();
    if(curr - last >= 1000000) {
        //
        // Only re-display when the second has changed.
        rtc_get_datetime(&dt);
        sprintf(tb, "%04d/%02d/%02d %02d:%02d:%02d", dt.year, dt.month, dt.day,
                dt.hour, dt.min, dt.sec);
        canvas->clear();
        font->render(tb, 5, 4, RGB::White);
    }
    last = curr;
    return true;
}
