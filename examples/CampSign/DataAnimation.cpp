#include "DataAnimation.h"

#include "pico/stdlib.h"
#include "pico/printf.h"

DataAnimation::DataAnimation(Canvas* canvas, Font* font, uint duration,
                             data_t *data)
    : TextAnimation(canvas, font, duration), data(data) {

        //
        // Make text chunks for each of the data elements
        char pop[8];
        char high[6];
        char low[6];
        char sr[10];
        char ss[10];
        sprintf(pop, "P: %2d", data->pop);
        sprintf(high, "H: %2d",   data->high);
        sprintf(low, "L: %2d",   data->low);
        sprintf(sr, "SR: %4s",   data->sunrise);
        sprintf(ss, "SS: %4s",   data->sunset);
        uint w1 = font->getWidth(pop);
        uint w2 = font->getWidth(high);
        uint w3 = font->getWidth(sr);
        uint x = 5;
        add(new TextElement(pop, x, 9, RGB::Blue));
        x += w1 + 1;
        add(new TextElement(high, x, 9, RGB::Red));
        x += w2 + 1;
        add(new TextElement(low, x, 9, RGB::Blue));
        x = 5;
        add(new TextElement(sr, x, 0, RGB::Yellow));
        x += w3 + 1;
        add(new TextElement(sr, x, 0, RGB::Yellow));

    }
