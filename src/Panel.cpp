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

void Panel::rotateLeft() {}

void Panel::rotateRight() {}