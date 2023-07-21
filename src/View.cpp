#include "View.h"

#include <stdio.h>

#include "Canvas.h"
#include "pico/platform.h"

void View::setCanvas(Canvas* canvas) { this->canvas = canvas; }

void View::add(Strip &strip) {
    uint np = strip.getNumPixels();

    if (np == width) {
        rows.push_back(Row(&strip, 0, width, StripDirection::FORWARDS));
        return;
    }

    if (np < width) {
        printf("Strip added with %d pixels, but width is %d", np, width);
        rows.push_back(Row(&strip, 0, np, StripDirection::FORWARDS));
        return;
    }

    if (np % width != 0) {
        printf("Strip added with %d pixels, non-integer multiple of width %d",
               np, width);
    }

    int start = 0;
    StripDirection dir = StripDirection::FORWARDS;
    while (np > 0) {
        rows.push_back(Row(&strip, start, width, dir));
        start += width;
        np -= width;
        if (dir == StripDirection::FORWARDS) {
            dir = StripDirection::BACKWARDS;
        } else {
            dir = StripDirection::FORWARDS;
        }
    }

    strips.push_back(strip);
}

void View::render() {
    //
    // Are we even on the canvas?
    if (canvas->viewX > canvas->width || canvas->viewY > canvas->height) {
        //
        // Everything off the canvas is black.
        for (auto r : rows) {
            r.strip->fill(RGB::Black);
            r.strip->show();
        }
        return;
    }

    //
    // Let's figure out the actual width and height that we will render, in
    // terms of width and height.
    uint rw = MIN(width, canvas->width - canvas->viewX);
    uint rh = MIN(rows.size(), canvas->height - canvas->viewY);

    //
    // We'll loop through the rows, rendering as we go. Note that we're assuming
    // that rows are added in order from the origin up.
    uint cy = canvas->viewY;
    uint rc = 0;
    for (auto r : rows) {
        if (r.dir == StripDirection::FORWARDS) {
            //
            // Forward, so we can use memcpy behavior. We'll start at the
            // provided position in the underlying strip for this row, and copy
            // out the number of pixels in our rendered width.
            uint dp = canvas->getPos(canvas->viewX, cy);
            r.strip->putPixels(&canvas->data[dp], r.start, rw);
        } else {
            //
            // No backwards memcpy, so here we are. we're going to add pixels
            // one-by-one. The view may be "hanging off the end" of the canvas,
            // so we want to make sure that we start at the first pixel that's
            // actually on the canvas when we start putting pixels in the strip.
            uint dp = canvas->getPos(canvas->viewX + rw, cy)-1;
            for (uint p = r.start + (width - rw); p < r.start + width; p++) {
                r.strip->putPixel(canvas->data[dp--], p);
            }
        }
        cy++;
        rc++;
        if (rc >= rh) {
            break;
        }
    }
    for (auto s : strips) {
        s.show();
    }
}
