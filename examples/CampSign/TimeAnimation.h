#ifndef TIMEANIMATION_H
#define TIMEANIMATION_H

#include "Animation.h"
#include "Font.h"
#include "pico/util/datetime.h"

#pragma once

class TimeAnimation : public Animation {

    protected:

    Font *font;
    datetime_t dt;
    char tb[20];

    void render();


    public:
    /// @brief Creates a data animation that will display the time and weather data.
    /// @param canvas the canvas we'll display on
    /// @param font the font we'll use for display
    TimeAnimation(Canvas *canvas, Font *font) : Animation(canvas), font(font) { };
    void init();
    bool step();

};
#endif
