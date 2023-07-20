#ifndef DATAANIMATION_H
#define DATAANIMATION_H

#include "TextAnimation.h"
#include "data.h"

#pragma once

class DataAnimation : public TextAnimation {
   protected:
    data_t *data;
    char pop[10];
    char high[10];
    char low[10];
    char sr[10];
    char ss[10];

   public:
    /// @brief Creates a data animation that will display the time and weather
    /// data.
    /// @param canvas the canvas we'll display on
    /// @param font the font we'll use for display
    /// @param duration how long to show the data for
    /// @param data the data to show
    DataAnimation(Canvas *canvas, Font *font, uint duration, data_t *data);

    ~DataAnimation();

    void display();
};
#endif
