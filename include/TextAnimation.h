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
    TextElement(const char *text, int x, int y, uint8_t color);
    const char *text;
    int startx;
    int starty;
    int x;
    int y;
    bool waiting;
    uint width;
    uint8_t color;
};

class TextAnimation : public Animation {
   protected:
    Font *font;
    std::vector<TextElement *> elements;
    RenderAngle angle = RENDER_0;

   public:
    /// @brief Creates a text "animation" that just displays the text for a
    /// given time.
    /// @param canvas
    /// @param font
    /// @param duration How long to display the text, in milliseconds.
    TextAnimation(Canvas *canvas, ColorMap *colorMap, Font *font);

    void add(TextElement *element);

    void setAngle(RenderAngle angle) { this->angle = angle; };

    /// @brief Initializes this animation, by rendering the text elements.
    void init() override;

    bool step() override;

};

#endif