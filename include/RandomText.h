#ifndef RANDOMTEXT_H
#define RANDOMTEXT_H

#pragma once

#include "TextAnimation.h"

/// @brief Shows a random text from a selection of the elements.
class RandomText : public TextAnimation {

    public:
    RandomText(Canvas *canvas, ColorMap *colorMap, Font *font, uint duration) : TextAnimation(canvas, colorMap, font, duration) {
    };

    void init();
};

#endif