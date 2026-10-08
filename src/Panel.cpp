#include "Panel.h"

bool Panel::set(uint x, uint y, const RGB& color) {
    if (x >= width || y >= height) return false;
    uint index;
    switch (origin) {
        case TopLeft:
            index = x * height + ((x % 2 == 0) ? (height - 1 - y) : y);
            break;
        case BottomRight:
            index = (width - 1 - x) * height +
                    ((x % 2 == 0) ? y : (height - 1 - y));
            break;
        case TopRight:
            index = (width - 1 - x) * height +
                    ((x % 2 == 0) ? (height - 1 - y) : y);
            break;
        case BottomLeft:
            index = x * height + ((x % 2 == 0) ? (height - 1 - y) : y);
            break;
        default:
            return false;
    }

    Strip::putPixel(color, index);
    return true;
}

const RGB& Panel::get(uint x, uint y) {
    uint index;
    switch (origin) {
        case TopLeft:
            index = x * height + ((x % 2 == 0) ? (height - 1 - y) : y);
            break;
        case BottomRight:
            index = (width - 1 - x) * height +
                    ((x % 2 == 0) ? y : (height - 1 - y));
            break;
        case TopRight:
            index = (width - 1 - x) * height +
                    ((x % 2 == 0) ? (height - 1 - y) : y);
            break;
        case BottomLeft:
            index = x * height + ((x % 2 == 0) ? (height - 1 - y) : y);
            break;
        default:
            return stripBlack;
    }
    return data[index];
}

void Panel::fill(const RGB& color) {
    RGB* d = data;
    for (int i = 0; i < numPixels; i++) {
        *d++ = color;
    }
}

void Panel::fillColumn(int x, const RGB& color) {
    if (x < 0 || x >= width) return;
    for (int y = 0; y < height; y++) {
        set(x, y, color);
    }
}

void Panel::fillRow(int y, const RGB& color) {
    if (y < 0 || y >= height) return;
    for (int x = 0; x < width; x++) {
        set(x, y, color);
    }
}

//
// The panel is wired serpentine, so neighboring columns run in opposite
// directions in the pixel data. Shifting the raw data by a column would flip
// every column upside down, so we move pixels by their (x, y) position instead.
void Panel::rotateLeft() {
    RGB tmp[height];
    for (int y = 0; y < height; y++) {
        tmp[y] = get(0, y);
    }
    for (int x = 0; x < width - 1; x++) {
        for (int y = 0; y < height; y++) {
            set(x, y, get(x + 1, y));
        }
    }
    for (int y = 0; y < height; y++) {
        set(width - 1, y, tmp[y]);
    }
}

void Panel::rotateRight() {
    RGB tmp[height];
    for (int y = 0; y < height; y++) {
        tmp[y] = get(width - 1, y);
    }
    for (int x = width - 1; x > 0; x--) {
        for (int y = 0; y < height; y++) {
            set(x, y, get(x - 1, y));
        }
    }
    for (int y = 0; y < height; y++) {
        set(0, y, tmp[y]);
    }
}
