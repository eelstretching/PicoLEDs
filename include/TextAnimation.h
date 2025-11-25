#ifndef TEXTANIMATION_H
#define TEXTANIMATION_H

#pragma once

#include <vector>

#include "Animator.h"
#include "Canvas.h"
#include "Font.h"

/// @brief A text element that can appear in a text animation.
class TextElement {
   public:
    TextElement(const char *text, uint x, uint y, uint8_t color);
    const char *text;
    uint x;
    uint y;
    uint8_t color;
};

class TextAnimation : public Animation {
   protected:
    /// @brief How long to display the canvas, in microseconds.
    uint duration;
    uint64_t start;
    Font *font;
    std::vector<TextElement *> elements;

   public:
    /// @brief Creates a text "animation" that just displays the text for a
    /// given time.
    /// @param canvas
    /// @param font
    /// @param duration How long to display the text, in milliseconds.
    TextAnimation(Canvas *canvas, ColorMap *colorMap, Font *font, uint duration);

    void add(TextElement *element);

    /// @brief Initializes this animation, by rendering the text elements.
    void init();

    bool step();

    int getFPSNeeded() {return 2;};
};

#endif