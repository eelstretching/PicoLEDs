#include "Panel.h"

#include <string.h>

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

void Panel::rotateLeft() {
    RGB tmp[height];
    memcpy(tmp, &data[0], height * sizeof(RGB));
    memmove(&data[0], &data[height], (numPixels - height) * sizeof(RGB));
    memcpy(&data[numPixels - height], tmp, height * sizeof(RGB));
}

void Panel::rotateRight() {
    RGB tmp[height];
    memcpy(tmp, &data[numPixels - height], height * sizeof(RGB));
    memmove(&data[height], &data[0], (numPixels - height) * sizeof(RGB));
    memcpy(&data[0], tmp, height * sizeof(RGB));
}