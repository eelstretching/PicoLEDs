#ifndef TEXTANIMATION_H
#define TEXTANIMATION_H

#pragma once

#include "Canvas.h"
#include "Font.h"
#include "Animator.h"

/// @brief A text element that can appear in a text animation.
class TextElement {
    public:
    TextElement(const char *text, uint x, uint y, RGB color);
    const char *text;
    uint x;
    uint y;
    RGB color;
    TextElement *next;
};

class TextAnimation : public Animation {
    protected:
    /// @brief How long to display the canvas, in microseconds.
    uint duration;
    uint64_t start;
    Font *font;
    TextElement *elements;
    TextElement *lastElement;

    public:
    /// @brief Creates a text "animation" that just displays the text for a given time.
    /// @param canvas 
    /// @param font
    /// @param duration How long to display the text, in milliseconds.
    TextAnimation(Canvas *canvas, Font *font, uint duration);

    void add(TextElement *element);

    /// @brief Initializes this animation, by rendering the text elements.
    void init();

    bool step();

};

#endif