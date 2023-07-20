#include "DataAnimation.h"

#include "pico/printf.h"
#include "pico/stdlib.h"

DataAnimation::DataAnimation(Canvas* canvas, Font* font, uint duration,
                             data_t* data)
    : TextAnimation(canvas, font, duration), data(data) {
    //
    // Make text chunks for each of the data elements
    sprintf(pop, "POP: %d", data->pop);
    sprintf(high, "HI: %d", data->high);
    sprintf(low, "LO: %d", data->low);
    sprintf(sr, "SR: %s", data->sunrise);
    sprintf(ss, "SS: %s", data->sunset);

    uint pp = 5;
    add(new TextElement(pop, pp, 9, RGB::Azure));
    printf("pop %s width: %d\n", pop, font->getWidth(pop));
    uint hp = pp + font->getWidth(pop) + 3;
    add(new TextElement(high, hp, 9, RGB::FireBrick));
    uint srp = hp + font->getWidth(high) + 3;
    printf("hp: %d srp: %d\n", hp, srp);
    add(new TextElement(sr, srp, 9, RGB::Gold));
    add(new TextElement(low, hp, 0, RGB::LightBlue));
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
