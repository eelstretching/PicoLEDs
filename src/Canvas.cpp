#include "Canvas.h"

#include <stdlib.h>
#include <string.h>

#include "pico/mem_ops.h"
#include "pico/platform.h"

void Row::set(int x, RGB color) {
    switch (dir) {
        case StripDirection::FORWARDS:
            strip->putPixel(color, start + x);
            break;
        case StripDirection::BACKWARDS:
            strip->putPixel(color, start + width - 1 - x);
            break;
    }
}

RGB Row::get(int x) {
    if (x <= 0 || x >= width) {
        //
        // Off the canvas is background color.
        return canvas->getBackground();
    }
    switch (dir) {
        case StripDirection::FORWARDS:
            return strip->get(start + x);
            break;
        case StripDirection::BACKWARDS:
            return strip->get(start + width - 1 - x);
            break;
        default:
            return canvas->getBackground();
    }
}

void Row::fill(RGB color) { strip->fill(color, start, width); }

int Row::copy(RGB *source, int p, int n) {
    int tc = MIN(width - p, n);
    int sp;
    int dp;
    switch (dir) {
        case StripDirection::FORWARDS:
            sp = 0;
            dp = start + p;
            while (sp < tc) {
                strip->getData()[dp++] = source[sp++];
            }
            break;
        case StripDirection::BACKWARDS:
            sp = tc - 1;
            dp = start + width - 1 - p;
            while (sp >= 0) {
                strip->getData()[dp++] = source[sp--];
            }
            break;
    }
    return tc;
}

void Row::copy(Row *other) {
    if (dir == other->dir) {
        //
        // Same direction, we can memcpy
        memcpy(&(strip->getData()[start]),
               &(other->strip->getData()[other->start]), width * sizeof(RGB));
        return;
    }

    //
    // Different directions, copy 1-1
    int end = other->start + width;
    int sp = other->start;
    int dp = start + width - 1;
    while (sp < end) {
        strip->getData()[dp--] = other->strip->getData()[sp++];
    }
}

Canvas::Canvas(uint width) : width(width) {
    numPixels = 0;
    rowBytes = width * sizeof(RGB);
}

void Canvas::add(Strip &strip) {
    uint np = strip.getNumPixels();

    if (np == width) {
        rows.push_back(Row(&strip, 0, width, StripDirection::FORWARDS, this));
        return;
    }

    if (np < width) {
        printf("Strip added with %d pixels, but width is %d", np, width);
        rows.push_back(Row(&strip, 0, np, StripDirection::FORWARDS, this));
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
        rows.push_back(Row(&strip, start, width, dir, this));
        start += width;
        np -= width;
        if (dir == StripDirection::FORWARDS) {
            dir = StripDirection::BACKWARDS;
        } else {
            dir = StripDirection::FORWARDS;
        }
    }

    numPixels += np;
    strips.push_back(strip);
}

void Canvas::setBackground(RGB b) { background = b; }

bool Canvas::set(int x, int y, RGB c) {
    if (x >= width || x < 0 || y >= rows.size() || y < 0) {
        //
        // Off the canvas.
        return false;
    }

    rows[y].set(x, c);
    return true;
}

RGB Canvas::get(uint x, uint y) { return rows[x].get(y); }

void Canvas::fillRow(uint row, RGB c) {
    if (row >= rows.size()) {
        return;
    }

    rows[row].fill(c);
}

void Canvas::copy(RGB *d, int n, int x, int y) {
    int cp = 0;
    int remain = n;
    for (int r = y; r < rows.size() && remain > 0; r++) {
        int copied = rows[r].copy(&d[cp], cp, remain);
        cp += copied;
        remain -= copied;
    }
}

void Canvas::fillColumn(uint col, RGB c) {
    if (col >= width) {
        return;
    }
    for (auto r : rows) {
        r.set(col, c);
    }
}

void Canvas::fill(RGB c) {
    for (auto r : rows) {
        r.fill(c);
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

void Canvas::scrollUp() { scrollUp(1, background); }

void Canvas::scrollUp(int n, RGB f) {
    if (n >= rows.size()) {
        //
        // Scrolled too much! Everything's black.
        fill(background);
        return;
    }
    //
    // We'll copy rows up 1 by 1 as our layout precludes one big memcpy.
    for (int sr = rows.size() - n - 1, dr = rows.size() - 1; sr >= 0;
         sr--, dr--) {
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
    if (n > rows.size()) {
        fill(background);
        return;
    }

    for (int sr = n, dr = 0; sr < rows.size(); sr++, dr++) {
        copyRow(sr, dr);
    }

    for (int i = 0; i < n; i++) {
        fillRow(rows.size() - i - 1, f);
    }
}

void Canvas::scrollLeft(int n, RGB f) {
    for (int col = n; col < width; col++) {
        copyColumn(col, col - n);
    }
    for (int col = width - n; col < width; col++) {
        fillColumn(col, f);
    }
}

void Canvas::scrollRight(int n, RGB f) {
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
    if (src > rows.size()) {
        fillRow(dst, background);
        return;
    }

    rows[src].copy(&rows[dst]);
}

void Canvas::copyColumn(int src, int dst) {
    if (src == dst) {
        return;
    }
    if (src > width) {
        fillColumn(dst, background);
    }
    for (auto row : rows) {
        row.set(dst, row.get(src));
    }
}

void Canvas::rotateUp() {
    //
    // A place to put the data from the top row.
    RGB tmp[width];
    for (int i = 0; i < width; i++) {
        tmp[i] = rows[rows.size() - 1].get(i);
    }
    scrollUp();
    rows[0].copy(tmp, rows[0].start, width);
}

void Canvas::rotateDown() {
    //
    // A place to put the data from the bottom row.
    RGB tmp[width];
    for (int i = 0; i < width; i++) {
        tmp[i] = rows[0].get(i);
    }
    scrollDown();
    rows[rows.size() - 1].copy(tmp, 0, width);
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

void Canvas::mirrorTopToBottom() { mirrorTopToBottom(rows.size() / 2); }

void Canvas::mirrorTopToBottom(int r) {
    if (r > rows.size()) {
        return;
    }

    for (int sp = r, dp = r - 1; sp < rows.size() && dp >= 0; sp++, dp--) {
        copyRow(sp, dp);
    }
}

void Canvas::mirrorBottomToTop() { mirrorBottomToTop(rows.size() / 2); }

void Canvas::mirrorBottomToTop(int r) {
    if (r > rows.size()) {
        return;
    }

    for (int sp = r - 1, dp = r; sp >= 0 && dp < rows.size(); sp--, dp++) {
        copyRow(sp, dp);
    }
}

void Canvas::clear() {
    for (auto row : rows) {
        row.fill(background);
    }
}

void Canvas::clear(uint x, uint y, uint w, uint h) {
    for (int i = y; i < y + h; i++) {
        for (int j = x; j < x + w; j++) {
            set(x, y, background);
        }
    }
}

void Canvas::clearRow(int row) {
    if (row < 0 || row > rows.size()) {
        return;
    }
    rows[row].fill(background);
}

void Canvas::clearColumn(int column) {
    if (column < 0 || column >= width) {
        return;
    }
    fillColumn(column, background);
}

void Canvas::shiftLeft(int x, int y, uint w, uint h, int n) {
    if (x >= width || y >= rows.size()) {
        return;
    }
    uint yf = MIN(y + h, rows.size());
    uint xf = MIN(x + w, width);
    for (int r = y; r < yf; r++) {
        Row row = rows[r];
        for (int j = MAX(x - n, 0); j < xf; j++) {
            row.set(j, row.get(j + n));
        }
        for (int j = xf; j < MIN(xf + n, width); j++) {
            row.set(j, background);
        }
    }
}

void Canvas::shiftRight(int x, int y, uint w, uint h, int n) {
    if (x >= width || y >= rows.size()) {
        return;
    }
    int yf = MIN(y + h, rows.size());
    int xsource = MIN(x + w - 1, width);
    int xdest = MIN(xsource + n, width);
    for (int row = y; row < yf; row++) {
        Row r = rows[row];
        for (int col = xsource; col >= x; col--) {
            r.set(col + n, r.get(col));
        }
        for (int col = x; col < x + n; col++) {
            r.set(col, background);
        }
    }
}

void Canvas::shiftUp(int x, int y, uint w, uint h, int n) {
    if (x >= width || y >= rows.size()) {
        return;
    }
    int yf = MIN(y + h, rows.size());
    int xf = MIN(x + w, width);
    for (int row = yf - 1; row >= y; row--) {
        Row src = rows[row - n];
        Row dst = rows[row];
        for (int j = x; j < xf; j++) {
            dst.set(j, src.get(j));
            src.set(j, background);
        }
    }
}

void Canvas::shiftDown(int x, int y, uint w, uint h, int n) {
    if (x >= width || y >= rows.size()) {
        return;
    }
    int yf = MIN(y + h - 1, rows.size());
    int xf = MIN(x + w - 1, width);
    for (int row = yf; row >= y; row--) {
        Row src = rows[row];
        Row dst = rows[row - n];
        for (int j = x; j < xf; j++) {
            dst.set(j, src.get(j));
            src.set(j, background);
        }
    }
}

void Canvas::show() {
    stats.start();
    for (auto s : strips) {
        s.show();
    }
    stats.finish();
}

StopWatch *Canvas::getStopWatch() { return &stats; }

void Canvas::printRect(int x, int y, int w, int h) {}

void Canvas::debugPrint() {}
