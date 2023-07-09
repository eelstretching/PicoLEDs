#ifndef DATAANIMATION_H
#define DATAANIMATION_H

#include "data.h"
#include "TextAnimation.h"

#pragma once

class DataAnimation : public TextAnimation {

    protected:
    data_t *data;

    public:
    /// @brief Creates a data animation that will display the time and weather data.
    /// @param canvas the canvas we'll display on
    /// @param font the font we'll use for display
    /// @param duration how long to show the data for
    /// @param data the data to show
    DataAnimation(Canvas *canvas, Font *font, uint duration, data_t *data);
};
#endif
