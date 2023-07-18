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

    //
    // Render the constant part, remember where it ends.
    lawPos = font->render(sis, 2, 4, RGB::ForestGreen);
    lawPos += 4;

    start = time_us_64();
    scrollY = 15;
    //
    // If we're doing a single element, pick a random one.
    if (single) {
        printf("Random selection\n");
        curr = random8(n);
    } else {
        printf("Everything\n");
        curr = 0;
    }
}

bool ScoutLaw::step() {
    uint64_t diff = time_us_64() - start;
    switch (state) {
        case SCROLLING:
            //
            // clear out the space, then render the current text at the given Y.
            canvas->clear(lawPos, 0, canvas->getWidth() - lawPos,
                          canvas->getHeight());
            font->render(law[curr], lawPos, scrollY, RGB::Gold);
            if (scrollY == 4) {
                //
                // At this postion, we need to wait for a bit.
                start = time_us_64();
                state = WAITING;
            } else if(scrollY + font->getFontHeight() == 0) {
                //
                // We've scrolled off screen.
                curr++;

                //
                // We're done our only one, or we're all done.
                if(single || curr >= n) {
                    return false;
                }

                //
                // Set up for the next one, if there isn't one, we're done.            
                scrollY = 15;
                state = SCROLLING;
            }
            scrollY--;
            break;
        case WAITING:
            if (diff >= scrollDuration) {
                state = SCROLLING;
            }
            break;
    }
    return true;
}
