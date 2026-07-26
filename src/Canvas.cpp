#include "Canvas.h"

#include <stdlib.h>
#include <string.h>

#include "ArrayColorMap.h"
#include "pico/platform.h"

void Row::set(int x, const RGB& color) {
    switch (dir) {
        case StripDirection::FORWARDS:
            strip->putPixel(color, start + x);
            break;
        case StripDirection::BACKWARDS:
            strip->putPixel(color, start + width - 1 - x);
            break;
    }
}

const RGB& Row::get(int x) {
    if (x < 0 || x >= width) {
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

void Row::fill(const RGB& color) { strip->fill(color, start, width); }

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

int Row::copy(RGB* source, int p, int n) {
    int tc = MIN(width - p, n);
    RGB *dp = &strip->getData()[p];
    for(int i = 0; i < tc; i++) {
        set(p++, *dp);
        dp++;
    }
    return tc;
}

void Row::copyOutData(RGB* dst) {
    memcpy(dst, &(strip->getData()[start]), width * sizeof(RGB));
}

void Row::copyInData(RGB* src) {
    memcpy(&(strip->getData()[start]), src, width * sizeof(RGB));
}

void Row::copy(Row* src) {
    if (dir == src->dir) {
        //
        // Same direction, we can memcpy
        memcpy(&(strip->getData()[start]), &(src->strip->getData()[src->start]), 
               width * sizeof(RGB));
        return;
    }

    //
    // Different directions, copy one by one.
    int sp = src->start;
    int end = src->start + width;
    int dp = (start + width - 1);
    RGB *srcd = &src->strip->getData()[sp];
    RGB *dstd = &strip->getData()[dp];
    while (sp < end) {
        *dstd-- = *srcd++;
        sp++;
    }
}

Canvas::Canvas(uint width) : width(width) {
    numPixels = 0;
    rowBytes = width * sizeof(RGB);
}

Canvas::~Canvas() {
    for(Row *row : rows) {
        delete row;
    }
}

void Canvas::add(Strip *strip) {
    uint np = strip->getNumPixels();

    if (np == width) {
        rows.push_back(new Row(strip, 0, width, StripDirection::FORWARDS, this));
        renderer.add(strip);
        return;
    }

    if (np < width) {
        printf("Strip added with %d pixels, but width is %d", np, width);
        rows.push_back(new Row(strip, 0, np, StripDirection::FORWARDS, this));
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
        rows.push_back(new Row(strip, start, width, dir, this));
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

void Canvas::setBackground(const RGB& background) {
    this->background = background;
}

bool Canvas::set(int x, int y, const RGB& color) {
    if (x >= width || x < 0 || y >= rows.size() || y < 0) {
        //
        // Off the canvas.
        return false;
    }

    rows[y]->set(x, color);
    return true;
}

const RGB& Canvas::get(uint x, uint y) { return rows[x]->get(y); }

void Canvas::fillRow(uint row, const RGB& color) {
    if (row >= rows.size() || row < 0) {
        return;
    }

    rows[row]->fill(color);
}

void Canvas::copy(RGB* d, int n, int x, int y) {
    int cp = 0;
    int remain = n;
    for (int r = y; r < rows.size() && remain > 0; r++) {
        int copied = rows[r]->copy(&d[cp], cp, remain);
        cp += copied;
        remain -= copied;
    }
}

void Canvas::fillColumn(uint col, const RGB& color) {
    if (col >= width || col < 0) {
        return;
    }
    for(auto row : rows) {
        row->set(col, color);
    }
}

void Canvas::fill(const RGB& color) {
    for(auto row : rows) {
        row->fill(color);
    }
}

void Canvas::fillRect(uint x0, uint y0, uint x1, uint y1, const RGB& color) {
    for (int i = x0; i <= x1; i++) {
        for (int j = y0; j <= y1; j++) {
            set(i, j, color);
        }
    }
}

void Canvas::drawLine(uint x0, uint y0, uint x1, uint y1, const RGB& color) {
    int dx = abs((int)x1 - (int)x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs((int)y1 - (int)y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2; /* error value e_xy */

    for (;;) { /* loop */
        set(x0, y0, color);
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

void Canvas::drawRect(uint x0, uint y0, uint x1, uint y1, const RGB& color) {
    drawLine(x0, y0, x1, y0, color);
    drawLine(x0, y0, x0, y1, color);
    drawLine(x0, y1, x1, y1, color);
    drawLine(x1, y1, x1, y0, color);
}

void Canvas::drawFilledRect(uint x0, uint y0, uint x1, uint y1, const RGB& lineColor,
                            const RGB& fillColor) {
    drawRect(x0, y0, x1, y1, lineColor);
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
            set(x, y, fillColor);
        }
    }
}

void Canvas::scrollUp() { scrollUp(1, background); }

void Canvas::scrollUp(int n, const RGB& fillColor) {
    if (n >= rows.size()) {
        //
        // Scrolled too much! Everything's background!.
        fill(fillColor);
        return;
    }
    //
    // We'll copy rows up 1 by 1 as our layout precludes one big memcpy.
    for (int sr = rows.size() - n - 1, dr = rows.size() - 1; sr >= 0;
         sr--, dr--) {
        copyRow(sr, dr);
    }
    for (int i = 0; i < n; i++) {
        fillRow(i, fillColor);
    }
}

void Canvas::scrollDown() { scrollDown(1, background); }

void Canvas::scrollDown(int n, const RGB& fillColor) {
    //
    // Scroll it all away?
    if (n > rows.size()) {
        fill(fillColor);
        return;
    }

    for (int sr = n, dr = 0; sr < rows.size(); sr++, dr++) {
        copyRow(sr, dr);
    }

    for (int i = 0; i < n; i++) {
        fillRow(rows.size() - i - 1, fillColor);
    }
}

void Canvas::scrollLeft(int n, const RGB& fillColor) {
    for (int col = n; col < width; col++) {
        copyColumn(col, col - n);
    }
    for (int col = width - n; col < width; col++) {
        fillColumn(col, fillColor);
    }
}

void Canvas::scrollRight(int n, const RGB& fillColor) {
    for (int col = width - n - 1; col >= 0; col--) {
        copyColumn(col, col + n);
    }
    for (int col = 0; col < n; col++) {
        fillColumn(col, fillColor);
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

    rows[dst]->copy(rows[src]);
}

void Canvas::copyColumn(int src, int dst) {
    if (src == dst) {
        return;
    }
    if (src > width) {
        fillColumn(dst, background);
    }
    int p = 0;
    for(auto row : rows) {
        row->set(dst, row->get(src));
    }
}

void Canvas::rotateRight() {
    for(auto row : rows) {
        row->rotateRight();
    }
}

void Canvas::rotateLeft() {
    for(auto row : rows) {
        row->rotateLeft();
    }
}

void Canvas::rotateUp() {
    //
    // A place to put the data from the top row.
    RGB tmp[width];
    Row* rotRow = rows[rows.size()-1];
    rotRow->copyOutData(tmp);
    scrollUp();
    rows[0]->copyInData(tmp);
}

void Canvas::rotateDown() {
    //
    // A place to put the data from the bottom row.
    RGB tmp[width];
    Row *rotRow = rows[0];
    rotRow->copyOutData(tmp);
    scrollDown();
    rows[rows.size() - 1]->copyInData(tmp);
}

void Canvas::flipHorizontal() {
    uint h = getHeight();
    for (int i = 0; i < width / 2; i++) {
        for (int j = 0; j < h; j++) {
            RGB temp = get(i, j);
            set(i, j, get(width - 1 - i, j));
            set(width - 1 - i, j, temp);
        }
    }
}

void Canvas::flipVertical() {
    uint h = getHeight();
    for (int i = 0; i < h / 2; i++) {
        for (int j = 0; j < width; j++) {
            RGB temp = get(j, i);
            set(j, i, get(j, h - 1 - i));
            set(j, h - 1 - i, temp);
        }
    }
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
    fill(background);
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
    rows[row]->fill(background);
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
        Row* row = rows[r];
        for (int j = MAX(x - n, 0); j < xf; j++) {
            row->set(j, row->get(j + n));
        }
        for (int j = xf; j < MIN(xf + n, width); j++) {
            row->set(j, background);
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
        Row* r = rows[row];
        for (int col = xsource; col >= x; col--) {
            r->set(col + n, r->get(col));
        }
        for (int col = x; col < x + n; col++) {
            r->set(col, background);
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
        Row* src = rows[row - n];
        Row* dst = rows[row];
        for (int j = x; j < xf; j++) {
            dst->set(j, src->get(j));
            src->set(j, background);
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
        Row* src = rows[row];
        Row* dst = rows[row - n];
        for (int j = x; j < xf; j++) {
            dst->set(j, src->get(j));
            src->set(j, background);
        }
    }
}

void Canvas::show(uint8_t brightness) {
    renderer.setBrightness(brightness);
    stats.start();
    renderer.render();
    stats.finish();
}

StopWatch* Canvas::getStats() { return &stats; }

void Canvas::printRect(int x, int y, int w, int h) {}

void Canvas::debugPrint() {}
