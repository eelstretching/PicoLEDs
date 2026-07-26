#include "TextAnimation.h"

#include "hardware/timer.h"
#include "pico/stdlib.h"
#include <inttypes.h>

TextElement::TextElement(const char* text, int x, int y, const RGB& color)
    : text(text), x(x), startx(x), y(y), starty(y), color(color) {}

TextAnimation::TextAnimation(Canvas* canvas, Font* font)
    : Animation(canvas, nullptr), font(font) {
}

void TextAnimation::add(TextElement* element) { elements.push_back(element); }

void TextAnimation::init() {
    canvas->clear();
    for (auto el : elements) {
        font->render(canvas, el->text, el->x, el->y, el->color, angle);
    }
}

bool TextAnimation::step() {
    return true;
}
