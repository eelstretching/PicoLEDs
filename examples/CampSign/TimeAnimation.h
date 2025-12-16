#ifndef TIMEANIMATION_H
#define TIMEANIMATION_H

#include "Animation.h"
#include "Font.h"
#include "pico/util/datetime.h"
#include "pico/aon_timer.h"

#pragma once

class TimeAnimation : public Animation {

    protected:

    Font *font;
    datetime_t dt;
    char tb[25];
    uint64_t start;
    uint64_t last;
    uint duration;

    void render();


    public:
    /// @brief Creates a data animation that will display the time and weather data.
    /// @param canvas the canvas we'll display on
    /// @param font the font we'll use for display
    /// @param duration the duration of the display, in milliseconds
    TimeAnimation(Canvas *canvas, Font *font, uint duration) : Animation(canvas, nullptr), font(font), duration(duration*1000) { 
        fps = 10;
    };
    void init();
    bool step();

};
#endif
