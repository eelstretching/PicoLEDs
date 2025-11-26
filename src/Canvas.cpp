#include "Canvas.h"

#include <stdlib.h>
#include <string.h>

#include "ArrayColorMap.h"
#include "pico/platform.h"

void Row::set(int x, uint8_t color) {
    switch (dir) {
        case StripDirection::FORWARDS:
            strip->putPixel(color, start + x);
            break;
        case StripDirection::BACKWARDS:
            strip->putPixel(color, start + width - 1 - x);
            break;
    }
}

uint8_t Row::get(int x) {
    if (x < 0 || x >= width) {
        //
        // Off the canvas is background color.
        return canvas->getBackgroundIndex();
    }
    switch (dir) {
        case StripDirection::FORWARDS:
            return strip->get(start + x);
            break;
        case StripDirection::BACKWARDS:
            return strip->get(start + width - 1 - x);
            break;
        default:
            return canvas->getBackgroundIndex();
    }
}

void Row::fill(uint8_t color) { strip->fill(color, start, width); }

void Row::rotateRight() {
    switch (dir) {
        case StripDirection::FORWARDS:
            strip->rotateRight(start, start + width);
            break;
        case StripDirection::BACKWARDS:
            strip->rotateLeft(start, start + width);
            break;
    }
}

void Row::rotateLeft() {
    switch (dir) {
        case StripDirection::FORWARDS:
            strip->rotateLeft(start, start + width);
            break;
        case StripDirection::BACKWARDS:
            strip->rotateRight(start, start + width);
            break;
    }
}

int Row::copy(uint8_t* source, int p, int n) {
    int tc = MIN(width - p, n);
    int sp;
    int dp;
    uint8_t* sd = strip->getData();
    switch (dir) {
        case StripDirection::FORWARDS:
            sp = 0;
            dp = start + p;
            while (sp < tc) {
                *sd++ = source[sp++];
            }
            break;
        case StripDirection::BACKWARDS:
            sp = tc - 1;
            dp = start + width - 1 - p;
            while (sp >= 0) {
                *sd++ = source[sp--];
            }
            break;
    }
    return tc;
}

void Row::copy(Row* dst) {
    if (dir == dst->dir) {
        //
        // Same direction, we can memcpy
        memcpy(&(dst->strip->getData()[dst->start]), &(strip->getData()[start]),
               width * sizeof(uint8_t));
        return;
    }

    //
    // Different directions, copy 1-1
    for (int i = 0; i < width; i++) {
        dst->set(i, get(i));
    }
}

Canvas::Canvas(uint width) : width(width) {
    numPixels = 0;
    rowBytes = width * sizeof(uint8_t);
    rows = new Row*[4];
    rowsLen = 4;
    nRows = 0;
}

Canvas::~Canvas() {
    for (int i = 0; i < nRows; i++) {
        delete rows[i];
    }
    delete rows;
}

ColorMap* Canvas::makeColorMap(uint8_t size) {
    colorMap = new ArrayColorMap(size + 1);
    colorMap->addColor(getBackground());
    return colorMap;
}

void Canvas::addRow(Row* row) {
    if (nRows >= rowsLen) {
        Row** tmp = rows;
        rows = new Row*[nRows * 2];
        memcpy(rows, tmp, rowsLen * sizeof(Row*));
    }
    rows[nRows++] = row;
}

void Canvas::add(Strip& strip) {
    uint np = strip.getNumPixels();

    if (np == width) {
        addRow(new Row(&strip, 0, width, StripDirection::FORWARDS, this));
        renderer.add(strip);
        return;
    }

    if (np < width) {
        printf("Strip added with %d pixels, but width is %d", np, width);
        addRow(new Row(&strip, 0, np, StripDirection::FORWARDS, this));
        renderer.add(strip);
        return;
    }

    if (np % width != 0) {
        printf("Strip added with %d pixels, non-integer multiple of width %d",
               np, width);
    }

    //
    // Our strip is a zig-zag of rows. Set the directions for each piece
    // appropriately. We're assuming that the first row's worth of pixels is in
    // the forwards direction.
    int start = 0;
    StripDirection dir = StripDirection::FORWARDS;
    while (np > 0) {
        addRow(new Row(&strip, start, width, dir, this));
        start += width;
        np -= width;
        if (dir == StripDirection::FORWARDS) {
            dir = StripDirection::BACKWARDS;
        } else {
            dir = StripDirection::FORWARDS;
        }
    }

    numPixels += np;
    renderer.add(strip);
}

void Canvas::setBackground(RGB& background) {
    colorMap->setBackground(background);
}

bool Canvas::set(int x, int y, uint8_t c) {
    if (x >= width || x < 0 || y >= nRows || y < 0) {
        //
        // Off the canvas.
        return false;
    }

    rows[y]->set(x, c);
    return true;
}

uint8_t Canvas::get(uint x, uint y) { return rows[x]->get(y); }

void Canvas::fillRow(uint row, uint8_t c) {
    if (row >= nRows || row < 0) {
        return;
    }

    rows[row]->fill(c);
}

void Canvas::copy(uint8_t* d, int n, int x, int y) {
    int cp = 0;
    int remain = n;
    for (int r = y; r < nRows && remain > 0; r++) {
        int copied = rows[r]->copy(&d[cp], cp, remain);
        cp += copied;
        remain -= copied;
    }
}

void Canvas::fillColumn(uint col, uint8_t c) {
    if (col >= width || col < 0) {
        return;
    }
    for (int i = 0; i < nRows; i++) {
        rows[i]->set(col, c);
    }
}

void Canvas::fill(uint8_t c) {
    for (int i = 0; i < nRows; i++) {
        rows[i]->fill(c);
    }
}

void Canvas::fillRect(uint x0, uint y0, uint x1, uint y1, uint8_t c) {
    for (int i = x0; i <= x1; i++) {
        for (int j = y0; j <= y1; j++) {
            set(i, j, c);
        }
    }
}

void Canvas::drawLine(uint x0, uint y0, uint x1, uint y1, uint8_t c) {
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

void Canvas::drawRect(uint x0, uint y0, uint x1, uint y1, uint8_t c) {
    drawLine(x0, y0, x1, y0, c);
    drawLine(x0, y0, x0, y1, c);
    drawLine(x0, y1, x1, y1, c);
    drawLine(x1, y1, x1, y0, c);
}

void Canvas::drawFilledRect(uint x0, uint y0, uint x1, uint y1, uint8_t l,
                            uint8_t f) {
    drawRect(x0, y0, x1, y1, l);
    if (x1 < x0) {
        uint tmp = x1;
        x1 = x0;
        x0 = tmp;
    }
    if (y1 < y0) {
        uint tmp = y1;
        y1 = y0;
        y0 = y1;
    }
    for (int x = x0 + 1; x < x1; x++) {
        for (int y = y0 + 1; y < y1; y++) {
            set(x, y, f);
        }
    }
}

void Canvas::scrollUp() { scrollUp(1, getBackgroundIndex()); }

void Canvas::scrollUp(int n, uint8_t f) {
    if (n >= nRows) {
        //
        // Scrolled too much! Everything's background!.
        fill(getBackgroundIndex());
        return;
    }
    //
    // We'll copy rows up 1 by 1 as our layout precludes one big memcpy.
    for (int sr = nRows - n - 1, dr = nRows - 1; sr >= 0; sr--, dr--) {
        printf("Copy from %d to %d\n", sr, dr);
        copyRow(sr, dr);
    }
    for (int i = 0; i < n; i++) {
        fillRow(i, f);
    }
}

void Canvas::scrollDown() { scrollDown(1, getBackgroundIndex()); }

void Canvas::scrollDown(int n, uint8_t f) {
    //
    // Scroll it all away?
    if (n > nRows) {
        fill(getBackgroundIndex());
        return;
    }

    for (int sr = n, dr = 0; sr < nRows; sr++, dr++) {
        copyRow(sr, dr);
    }

    for (int i = 0; i < n; i++) {
        fillRow(nRows - i - 1, f);
    }
}

void Canvas::scrollLeft(int n, uint8_t f) {
    for (int col = n; col < width; col++) {
        copyColumn(col, col - n);
    }
    for (int col = width - n; col < width; col++) {
        fillColumn(col, f);
    }
}

void Canvas::scrollRight(int n, uint8_t f) {
    for (int col = width - n - 1; col >= 0; col--) {
        copyColumn(col, col + n);
    }
    for (int col = 0; col < n; col++) {
        fillColumn(col, f);
    }
}

void Canvas::copyRow(int src, int dst) {
    if (src == dst) {
        //
        // Don't copy a row onto itself.
        return;
    }

    //
    // Everything's black off the canvas.
    if (src > nRows) {
        fillRow(dst, getBackgroundIndex());
        return;
    }

    rows[src]->copy(rows[dst]);
}

void Canvas::copyColumn(int src, int dst) {
    if (src == dst) {
        return;
    }
    if (src > width) {
        fillColumn(dst, getBackgroundIndex());
    }
    for (int i = 0; i < nRows; i++) {
        rows[i]->set(dst, rows[i]->get(src));
    }
}

void Canvas::rotateRight() {
    for (int i = 0; i < nRows; i++) {
        rows[i]->rotateRight();
    }
}

void Canvas::rotateLeft() {
    for (int i = 0; i < nRows; i++) {
        rows[i]->rotateLeft();
    }
}

void Canvas::rotateUp() {
    //
    // A place to put the data from the top row.
    uint8_t tmp[width];
    uint8_t from = nRows - 1;
    Row* rotRow = rows[from];
    for (int i = 0; i < width; i++) {
        tmp[i] = rows[from]->get(i);
    }
    scrollUp();
    rows[0]->copy(tmp, rows[0]->start, width);
}

void Canvas::rotateDown() {
    //
    // A place to put the data from the bottom row.
    uint8_t tmp[width];
    for (int i = 0; i < width; i++) {
        tmp[i] = rows[0]->get(i);
    }
    scrollDown();
    rows[nRows - 1]->copy(tmp, 0, width);
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
    // We're just going to copy columns until we get to the given column.
    for (int s = 0, d = width - 1; s < c && d >= c; s++, d--) {
        copyColumn(s, d);
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

    for (int s = width - 1, d = 0; s >= c && d < c; s--, d++) {
        copyColumn(s, d);
    }
}

void Canvas::mirrorTopToBottom() { mirrorTopToBottom(nRows / 2); }

void Canvas::mirrorTopToBottom(int r) {
    if (r > nRows) {
        return;
    }

    for (int sp = r, dp = r - 1; sp < nRows && dp >= 0; sp++, dp--) {
        copyRow(sp, dp);
    }
}

void Canvas::mirrorBottomToTop() { mirrorBottomToTop(nRows / 2); }

void Canvas::mirrorBottomToTop(int r) {
    if (r > nRows) {
        return;
    }

    for (int sp = r - 1, dp = r; sp >= 0 && dp < nRows; sp--, dp++) {
        copyRow(sp, dp);
    }
}

void Canvas::clear() {
    for (int i = 0; i < nRows; i++) {
        rows[i]->fill(getBackgroundIndex());
    }
}

void Canvas::clear(uint x, uint y, uint w, uint h) {
    for (int i = y; i < y + h; i++) {
        for (int j = x; j < x + w; j++) {
            set(x, y, getBackgroundIndex());
        }
    }
}

void Canvas::clearRow(int row) {
    if (row < 0 || row > nRows) {
        return;
    }
    rows[row]->fill(getBackgroundIndex());
}

void Canvas::clearColumn(int column) {
    if (column < 0 || column >= width) {
        return;
    }
    fillColumn(column, getBackgroundIndex());
}

void Canvas::shiftLeft(int x, int y, uint w, uint h, int n) {
    if (x >= width || y >= nRows) {
        return;
    }
    uint yf = MIN(y + h, nRows);
    uint xf = MIN(x + w, width);
    for (int r = y; r < yf; r++) {
        Row* row = rows[r];
        for (int j = MAX(x - n, 0); j < xf; j++) {
            row->set(j, row->get(j + n));
        }
        for (int j = xf; j < MIN(xf + n, width); j++) {
            row->set(j, getBackgroundIndex());
        }
    }
}

void Canvas::shiftRight(int x, int y, uint w, uint h, int n) {
    if (x >= width || y >= nRows) {
        return;
    }
    int yf = MIN(y + h, nRows);
    int xsource = MIN(x + w - 1, width);
    int xdest = MIN(xsource + n, width);
    for (int row = y; row < yf; row++) {
        Row* r = rows[row];
        for (int col = xsource; col >= x; col--) {
            r->set(col + n, r->get(col));
        }
        for (int col = x; col < x + n; col++) {
            r->set(col, getBackgroundIndex());
        }
    }
}

void Canvas::shiftUp(int x, int y, uint w, uint h, int n) {
    if (x >= width || y >= nRows) {
        return;
    }
    int yf = MIN(y + h, nRows);
    int xf = MIN(x + w, width);
    for (int row = yf - 1; row >= y; row--) {
        Row* src = rows[row - n];
        Row* dst = rows[row];
        for (int j = x; j < xf; j++) {
            dst->set(j, src->get(j));
            src->set(j, getBackgroundIndex());
        }
    }
}

void Canvas::shiftDown(int x, int y, uint w, uint h, int n) {
    if (x >= width || y >= nRows) {
        return;
    }
    int yf = MIN(y + h - 1, nRows);
    int xf = MIN(x + w - 1, width);
    for (int row = yf; row >= y; row--) {
        Row* src = rows[row];
        Row* dst = rows[row - n];
        for (int j = x; j < xf; j++) {
            dst->set(j, src->get(j));
            src->set(j, getBackgroundIndex());
        }
    }
}

void Canvas::show() {
    stats.start();
    renderer.render(colorMap);
    stats.finish();
}

StopWatch* Canvas::getStats() { return &stats; }

void Canvas::printRect(int x, int y, int w, int h) {}

void Canvas::debugPrint() {}
