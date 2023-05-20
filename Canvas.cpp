#include "Canvas.h"

#include <stdlib.h>
#include <string.h>

#include "View.h"
#include "pico/platform.h"

Canvas::Canvas(uint width, uint height) : width(width), height(height) {
    numPixels = width * height;
    rowBytes = width * sizeof(RGB);
    data = new RGB[numPixels];
}

void Canvas::setBackground(RGB b) { background = b; }

void Canvas::set(uint x, uint y, RGB c) {
    if (x >= width || y >= height) {
        //
        // Off the canvas.
        return;
    }

    data[getPos(x, y)] = c;
}

void Canvas::fill(uint row, RGB c) {
    if (row >= height) {
        return;
    }
    uint pos = getPos(0, row);
    for (int i = 0; i < width; i++) {
        data[pos++] = c;
    }
}

void Canvas::fill(RGB c) {
    for (int i = 0; i < numPixels; i++) {
        data[i] = c;
    }
}

void Canvas::scrollUp() { scrollUp(1, background); }

void Canvas::scrollUp(uint n, RGB f) {
    if (n >= height) {
        //
        // Scrolled too much! Everything's black.
        fill(background);
        return;
    }
    //
    // We'll copy rows up 1 by 1 as our layout precludes one big memcpy.
    for (int sr = height - n - 1, dr = height - 1; sr >= 0; sr--, dr--) {
        copyRow(sr, dr);
    }
    for(int i = 0; i < n; i++) {
        fill(i, f);
    }
}

void Canvas::scrollDown() { scrollDown(1, background); }

void Canvas::scrollDown(int n, RGB f) {
    //
    // Scorll it all away?
    if (n > height) {
        fill(background);
        return;
    }

    for(int sr = n, dr = 0; sr < height; sr++, dr++) {
        copyRow(sr, dr);
    }

    for(int i = 0; i < n; i++) {
        fill(height - i - 1, f);
    }           
}

void Canvas::copyRow(uint src, uint dst) {
    printf("Copying %d onto %d\n", src, dst);
    if (src == dst) {
        //
        // Don't copy a row onto itself.
        return;
    }

    //
    // Everything's black off the canvas.
    if (src > height) {
        fill(dst, background);
        return;
    }

    //
    // We can memcpy individual rows, as there's no overlap.
    int sp = getPos(0, src);
    int tp = getPos(0, dst);
    memcpy(&data[tp], &data[sp], rowBytes);
}

void Canvas::rotateUp() {
    //
    // A place to put the data from the top row.
    RGB tmp[width];
    printf("Rotate up tmp is %d bytes, rowBytes: %d\n", sizeof(tmp), rowBytes);
    memcpy(&tmp[0], &data[getPos(0, height - 1)], rowBytes);
    scrollUp();
    memcpy(&data[0], &tmp[0], rowBytes);
}

void Canvas::rotateDown() {
    //
    // A place to put the data from the bottom row.
    RGB tmp[width];
    printf("Rotate down tmp is %d bytes, rowBytes: %d\n", sizeof(tmp), rowBytes);
    memcpy(&tmp[0], &data[0], rowBytes);
    scrollDown();
    memcpy(&data[getPos(0, height - 1)], &tmp[0], rowBytes);
}

void Canvas::mirrorLeftToRight() {
    mirrorLeftToRight((width / 2) - (width % 2));
}

void Canvas::mirrorLeftToRight(int c) {
    if (c >= width) {
        //
        // Can't mirror the whole thing, bro.
        return;
    }

    //
    // This is the naive implementation.

    //
    // How many pixels are we going to mirror? We'll include the column that
    // we're mirroring through. Note that we need to account for the case when
    // the mirror column is beyond the halfway mark, in which case we only want
    // to mirror to the end of the row!
    int n = MAX(c + 1, width - c);
    for (int i = 0; i < height; i) {
        //
        // We'll start mirroring at the column itself and mirror onto the next
        // pixel over.
        int sp = getPos(c, i);
        int dp = getPos(c + 1, i);
        for (int j = 0; j < n; j++) {
            data[dp++] = data[sp--];
        }
    }
}

void Canvas::mirrorRightToLeft() {
    mirrorRightToLeft((width / 2) - (width % 2));
}

void Canvas::mirrorRightToLeft(int c) {
    if (c >= width) {
        //
        // Can't mirror the whole thing, bro.
        return;
    }

    //
    // This is the naive implementation.

    //
    // How many pixels are we going to mirror? We'll include the column that
    // we're mirroring through. Note that we need to account for the case when
    // the mirror column is beyond the halfway mark, in which case we only want
    // to mirror to the end of the row!
    int n = MAX(width - c - 1, c+1);
    for (int i = 0; i < height; i) {
        //
        // We'll start mirroring at the column itself and mirror onto the next
        // pixel over.
        int sp = getPos(c+1, i);
        int dp = getPos(c, i);
        for (int j = 0; j < n; j++) {
            data[dp--] = data[sp++];
        }
    }
}

uint Canvas::getPos(uint x, uint y) { return y * width + x; }

void Canvas::clear() {
    for (int i = 0; i < numPixels; i++) {
        data[i] = background;
    }
}

void Canvas::setView(View *v, uint x, uint y) {
    view = v;
    view->setCanvas(this);
    viewX = x;
    viewY = y;
}

void Canvas::show() {
    stats.start();
    view->render();
    stats.finish();
}

StopWatch *Canvas::getStopWatch() { return &stats; }

void Canvas::debugPrint() {

    char b[30];
    for(int i = 0; i < height; i++) {
        int p =getPos(0, i);
        printf("row %d pos %d %s\n", i, p, data[p].toString(b, 30));
    }
}
