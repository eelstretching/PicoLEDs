#include "DataAnimation.h"

#include "pico/printf.h"
#include "pico/stdlib.h"

DataAnimation::DataAnimation(Canvas* canvas, Font* font, uint duration,
                             data_t* data)
    : TextAnimation(canvas, font, duration), data(data) {
    //
    // Make text chunks for each of the data elements
    sprintf(pop, "P: %d", data->pop);
    sprintf(high, "H: %d", data->high);
    sprintf(low, "L: %d", data->low);
    sprintf(sr, "SR: %s", data->sunrise);
    sprintf(ss, "SS: %s", data->sunset);

    uint pp = 5;
    add(new TextElement(pop, pp, 9, RGB::Blue));
    uint hp = pp + font->getWidth(pop) + 3;
    add(new TextElement(high, hp, 9, RGB::Red));
    uint srp = hp + font->getWidth(high) + 3;
    add(new TextElement(sr, srp, 9, RGB::Yellow));
    add(new TextElement(low, hp, 0, RGB::Blue));
    add(new TextElement(ss, srp, 0, RGB::Gray));
}

DataAnimation::~DataAnimation() {
    for (auto el : elements) {
        delete el;
    }
    elements.clear();   
}

void DataAnimation::display() {
    printf("%s\n", pop);
    printf("%s\n", high);
    printf("%s\n", low);
    printf("%s\n", sr);
    printf("%s\n", ss);
}
