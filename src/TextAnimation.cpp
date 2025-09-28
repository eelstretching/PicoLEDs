#include "TextAnimation.h"

#include "hardware/timer.h"
#include "pico/stdlib.h"
#include <inttypes.h>

TextElement::TextElement(const char* text, uint x, uint y, uint8_t color)
    : text(text), x(x), y(y), color(color) {}

TextAnimation::TextAnimation(Canvas* canvas, Font* font, uint duration)
    : Animation(canvas), font(font) {
    //
    // Input is milliseconds, but we'll time in microseconds.
    this->duration = duration * 1000;
}

void TextAnimation::add(TextElement* element) { elements.push_back(element); }

void TextAnimation::init() {
    start = time_us_64();
    canvas->clear();
    for (auto el : elements) {
        font->render(el->text, el->x, el->y, el->color);
    }
}

bool TextAnimation::step() {
    return time_us_64() - start < duration;
}
