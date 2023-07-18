#include "RandomText.h"

#include "math8.h"

void RandomText::init() {
    start = time_us_64();
    canvas->clear();
    TextElement *el = elements[random8(0, elements.size())];
    font->render(el->text, el->x, el->y, el->color);
}
