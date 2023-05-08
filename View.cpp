#include "View.h"

#include <stdio.h>

#include "pico/platform.h"

#include "Canvas.h"

void View::setCanvas(Canvas* canvas, uint x, uint y) {
    this->canvas = canvas;
    this->x = x;
    this->y = y;
}

void View::add(Strip strip) {
    uint np = strip.getNumPixels();

    if (np == width) {
        rows.push_back(Row(&strip, 0, width, Direction::FORWARDS));
        return;
    }

    if (np < width) {
        printf("Strip added with %d pixels, but width is %d", np, width);
        rows.push_back(Row(&strip, 0, np, Direction::FORWARDS));
        return;
    }

    if (np % width != 0) {
        printf("Strip added with %d pixels, non-integer multiple of width %d",
               np, width);
    }

    int start = 0;
    Direction dir = Direction::FORWARDS;
    while (np > 0) {
        rows.push_back(Row(&strip, start, width, dir));
        start += width;
        np -= width;
        if (Direction::FORWARDS) {
            dir = Direction::BACKWARDS;
        } else {
            dir = Direction::FORWARDS;
        }
    }
}

void View::render() {
    //
    // Are we even on the canvas?
    if (x > canvas->width || y > canvas->height) {
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
    uint rh = MIN(height, canvas->height - canvas->viewY);

    //
    // We'll loop through the rows, rendering as we go. Note that we're assuming
    // that rows are added in order from the origin up.
    uint cy = y;
    for (auto r : rows) {
        if (r.dir == Direction::FORWARDS) {
            //
            // Forward, so we can use memcpy behavior. We'll start at the
            // provided position in the underlying strip for this row, and copy
            // out the number of pixels in our rendered width.
            r.strip->putPixels(r.start, &canvas->data[canvas->getPos(x, cy)],
                               rw);
        } else {
            //
            // No backwards memcpy, so here we are. we're going to add pixels
            // one-by-one. The view may be "hanging off the end" of the canvas,
            // so we want to make sure that we start at the first pixel that's
            // actuall on the canvas when we start putting pixels in the strip.
            uint dp = canvas->getPos(canvas->viewX + rw, cy);
            for (uint p = r.start + (width - rw); p < r.start + width; p++) {
                r.strip->putPixel(p, canvas->data[dp--]);
            }
        }
        cy++;
    }
}
