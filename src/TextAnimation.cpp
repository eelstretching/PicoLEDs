#include "TextAnimation.h"

TextElement::TextElement(const char* text, uint x, uint y, RGB color) : text(text), x(x), y(y), color(color) {}

TextAnimation::TextAnimation(Canvas* canvas, Font* font, uint duration) : Animation(canvas), font(font) {
    elements = NULL;
    lastElement = NULL;
    //
    // Input is milliseconds, but we'll time in microseconds.
    this->duration = duration * 1000;
}

void TextAnimation::add(TextElement* element) {
    element->next = NULL;
    if(elements == NULL) {
        elements = element;
        lastElement = element;
    } else {
        lastElement->next = element;
        lastElement = element;
    }
}

void TextAnimation::init() {
    start = time_us_64();
    TextElement *curr = elements;
    while(curr != NULL) {
        font->render(curr->text, curr->x, curr->y, curr->color);
        curr = curr->next;
    }
}

bool TextAnimation::step() {
    return time_us_64() - start < duration;
}

