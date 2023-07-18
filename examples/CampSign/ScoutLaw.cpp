#include "ScoutLaw.h"

#include "color.h"
#include "math8.h"

#define SCROLLING 1
#define WAITING 2

ScoutLaw::ScoutLaw(Canvas* canvas, Font* font)
    : Animation(canvas), font(font) {}

void ScoutLaw::init() {

    canvas->clear();

    single = random8(100) < 50;
    state = SCROLLING;

    int w = font->getWidth(sis);

    //
    // Render the constant part, remember where it ends.
    font->render(sis, (canvas->getHeight() - w) / 2, 8, RGB::ForestGreen);

    //
    // If we're doing a single element, pick a random one.
    if (single) {
        printf("Random selection\n");
        curr = random8(n);
    } else {
        printf("Everything\n");
        curr = 0;
    }

    cw = font->getWidth(law[curr]);
    mid = (canvas->getWidth() - cw) / 2;
    scrollX = 0 - cw;

    start = time_us_64();
}

bool ScoutLaw::step() {
    uint64_t diff = time_us_64() - start;
    switch (state) {
        case SCROLLING:
            //
            // clear out the space, then render the current text at the given Y.
            for(int i = 0; i < font->getFontHeight(); i++) {
                canvas->clearRow(i);
            }
            font->render(law[curr], scrollX, 0, RGB::Gold);
            if (scrollX == mid) {
                //
                // At this postion, we need to wait for a bit.
                start = time_us_64();
                state = WAITING;
            } else if (scrollX >= (int) canvas->getWidth()) {
                //
                // We've scrolled off screen.
                curr++;

                //
                // We're done our only one, or we're all done.
                if (single || curr >= n) {
                    return false;
                }

                //
                // Set up for the next one, if there isn't one, we're done.
                cw = font->getWidth(law[curr]);
                mid = (canvas->getWidth() - cw) / 2;
                scrollX = 0 - cw;
                state = SCROLLING;
            } else {
                scrollX++;
            }
            break;
        case WAITING:
            if (diff >= scrollDuration) {
                state = SCROLLING;
            }
            break;
    }
    return true;
}
