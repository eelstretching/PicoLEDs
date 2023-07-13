#include "Canvas.h"

#include <stdlib.h>
#include <string.h>

#include "View.h"
#include "pico/platform.h"
#include "pico/mem_ops.h"

Canvas::Canvas(uint width, uint height) : width(width), height(height) {
    numPixels = width * height;
    rowBytes = width * sizeof(RGB);
    data = new RGB[numPixels];
    clear();
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

RGB Canvas::get(uint x, uint y) { return data[getPos(x, y)]; }

void Canvas::fillRow(uint row, RGB c) {
    if (row >= height) {
        return;
    }
    uint pos = getPos(0, row);
    for (int i = 0; i < width; i++) {
        data[pos++] = c;
    }
}

void Canvas::copy(RGB *d, int n, int x, int y) {
    int dp = getPos(x, y);
    int sp = 0;
    for (dp = getPos(x, y), sp = 0; sp < n && dp < numPixels; sp++, dp++) {
        data[dp] = d[sp];
    }
}

void Canvas::fillColumn(uint col, RGB c) {
    if (col >= width) {
        return;
    }
    uint pos = getPos(col, 0);
    for (int i = 0; i < height; i++) {
        data[pos] = c;
        pos += width;
    }
}

void Canvas::fill(RGB c) {
    for (int i = 0; i < numPixels; i++) {
        data[i] = c;
    }
}

void Canvas::drawLine(uint x0, uint y0, uint x1, uint y1, RGB c) {
    int dx = abs((int)x1 - (int)x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs((int)y1 - (int)y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2; /* error value e_xy */

    for (;;) { /* loop */
        set(x0, y0, c);
        if (x0 == x1 && y0 == y1) {
            break;
        }
        e2 = err + err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        } /* e_xy+e_x > 0 */
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        } /* e_xy+e_y < 0 */
    }
}

void Canvas::drawRect(uint x0, uint y0, uint x1, uint y1, RGB c) {
    drawLine(x0, y0, x1, y0, c);
    drawLine(x0, y0, x0, y1, c);
    drawLine(x0, y1, x1, y1, c);
    drawLine(x1, y1, x1, y0, c);
}

void Canvas::drawFilledRect(uint x0, uint y0, uint x1, uint y1, RGB l, RGB f) {
    drawRect(x0, y0, x1, y1, l);
    if(x1 < x0) {
        uint tmp = x1;
        x1 = x0; 
        x0 = tmp;
    }
    if(y1 < y0) {
        uint tmp = y1;
        y1 = y0;
        y0 = y1;
    }
    for(int x = x0 + 1; x < x1; x++) {
        for(int y = y0 + 1; y < y1; y++) {
            set(x, y, f);
        }
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
    for (int i = 0; i < n; i++) {
        fillRow(i, f);
    }
}

void Canvas::scrollDown() { scrollDown(1, background); }

void Canvas::scrollDown(int n, RGB f) {
    //
    // Scroll it all away?
    if (n > height) {
        fill(background);
        return;
    }

    for (int sr = n, dr = 0; sr < height; sr++, dr++) {
        copyRow(sr, dr);
    }

    for (int i = 0; i < n; i++) {
        fillRow(height - i - 1, f);
    }
}

void Canvas::copyRow(uint src, uint dst) {
    if (src == dst) {
        //
        // Don't copy a row onto itself.
        return;
    }

    //
    // Everything's black off the canvas.
    if (src > height) {
        fillRow(dst, background);
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
    memcpy(&tmp[0], &data[getPos(0, height - 1)], rowBytes);
    scrollUp();
    memcpy(&data[0], &tmp[0], rowBytes);
}

void Canvas::rotateDown() {
    //
    // A place to put the data from the bottom row.
    RGB tmp[width];
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
    int n = MAX(c, width - c);
    for (int i = 0; i < height; i++) {
        //
        // We'll start mirroring at the column itself and mirror onto the next
        // pixel over.
        int sp = getPos(c-1, i);
        int dp = getPos(c, i);
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
    int n = MAX(width - c - 1, c);
    for (int i = 0; i < height; i++) {
        //
        // We'll start mirroring at the column itself and mirror onto the next
        // pixel over.
        int sp = getPos(c, i);
        int dp = getPos(c-1, i);
        for (int j = 0; j < n; j++) {
            data[dp--] = data[sp++];
        }
    }
}

void Canvas::mirrorTopToBottom() { mirrorTopToBottom(height / 2); }

void Canvas::mirrorTopToBottom(int r) {
    if (r > height) {
        return;
    }

    for (int sp = r, dp = r - 1; sp < height && dp >= 0; sp++, dp--) {
        copyRow(sp, dp);
    }
}

void Canvas::mirrorBottomToTop() { mirrorBottomToTop(height / 2); }

void Canvas::mirrorBottomToTop(int r) {
    if (r > height) {
        return;
    }

    for (int sp = r - 1, dp = r; sp >= 0 && dp < height; sp--, dp++) {
        copyRow(sp, dp);
    }
}

uint Canvas::getPos(uint x, uint y) { return y * width + x; }

void Canvas::clear() {
    for (int i = 0; i < numPixels; i++) {
        data[i] = background;
    }
}

void Canvas::clearRow(uint row) {
    for(int dp = getPos(0, row), i = 0; i < width; i++, dp++) {
        data[dp] = background;
    }
}

void Canvas::clearColumn(uint column) {
    for(int dp = getPos(column, 0), i = 0; i < height; i++, dp += width) {
        data[dp] = background;
    }
}

void Canvas::setView(View *v, uint x, uint y) {
    view = v;
    view->setCanvas(this);
    viewX = x;
    viewY = y;
}

void Canvas::shiftLeft(uint x, uint y, uint w, uint h, int n) {
    if (x >= width || y >= height) {
        return;
    }
    uint yf = MIN(y + h, height);
    uint xf = MIN(x + w, width);
    uint xn = n > x ? 0 : x - n;
    for (int row = y; row < yf; row++) {
        int sp = getPos(x, row);
        int ep = getPos(xf, row);
        int dp = getPos(xn, row);
        for (int i = sp; i < ep; i++) {
            data[dp++] = data[sp++];
        }
        dp = getPos(xf - n, row);
        //
        // Fill in the pixels we vacated with the background.
        for (int i = 0; i < n && dp < numPixels; i++) {
            data[dp++] = background;
        }
    }
}

void Canvas::shiftRight(uint x, uint y, uint w, uint h, int n) {
    if (x >= width || y >= height) {
        return;
    }
    uint yf = MIN(y + h, height);
    uint xf = MIN(x + w, width);
    uint xn = MIN(xf + n, width);
    for (int row = y; row < yf; row++) {
        int sp = getPos(xf - 1, row);
        int dp = getPos(xn - 1, row);
        int ep = getPos(x, row);
        for (int i = sp; i >= ep; i--) {
            data[dp--] = data[sp--];
        }
        dp = getPos(x, row);
        //
        // Fill in the pixels we vacated with the background.
        for (int i = 0; i < n && dp < numPixels; i++) {
            data[dp++] = background;
        }
    }
}

void Canvas::shiftUp(uint x, uint y, uint w, uint h, int n) {
    if (x >= width || y >= height) {
        return;
    }
    int yf = MIN(y + h, height);
    for (int row = yf - 1; row >= (int)y; row--) {
        int sp = getPos(x, row);
        int dp = getPos(x, row + n);
        memcpy(&data[dp], &data[sp], w * sizeof(RGB));
    }
    //
    // Fill the rows we vacated with the background.
    for (int row = y; row < y + n; row++) {
        int dp = getPos(x, row);
        for (int j = 0; j < w; j++) {
            data[dp++] = background;
        }
    }
}

void Canvas::shiftDown(uint x, uint y, uint w, uint h, int n) {
    if (x >= width || y >= height) {
        return;
    }
    int yf = MIN(y + h, height);
    int xf = MIN(x + w, width);
    for (int row = y; row < yf; row++) {
        int sp = getPos(x, row);
        int dp = getPos(x, row - n);
        memcpy(&data[dp], &data[sp], w * sizeof(RGB));
    }
    for (int row = yf - 1; row >= yf - n; row--) {
        int dp = getPos(x, row);
        for (int i = 0; i < w; i++) {
            data[dp++] = background;
        }
    }
}

void Canvas::show() {
    stats.start();
    view->render();
    stats.finish();
}

StopWatch *Canvas::getStopWatch() { return &stats; }

void Canvas::printRect(int x, int y, int w, int h) {
    char b[30];
    printf("(%d,%d\n", x, y);
    for (int i = 0; i < h; i++) {
        int dp = getPos(x, y + i);
        for (int j = 0; j < w; j++) {
            printf("%s ", data[dp++].toString(b, 30));
        }
        printf("\n");
    }
}

void Canvas::debugPrint() {
    char b[30];
    for (int i = 0; i < height; i++) {
        int p = getPos(0, i);
        printf("row %d pos %d %s\n", i, p, data[p].toString(b, 30));
    }
}
