#include "ArrayColorMap.h"
#include <cstring>

ArrayColorMap::ArrayColorMap() : ArrayColorMap(16) {
    
}

ArrayColorMap::ArrayColorMap(uint8_t size) : ColorMap(RGB::Black), size(size), p(0) {
    entries = new RGB[size];
    for (uint8_t i = 0; i < size; i++) {
        entries[i] = RGB::Black;
    }
    p = 0;
}

ArrayColorMap::ArrayColorMap(std::initializer_list<RGB> rhs)
    : ColorMap(RGB::Black), p(rhs.size()), size(rhs.size()) {
    entries = new RGB[rhs.size()];
    int i = 0;
    for (auto c : rhs) {
        entries[i++] = c;
    }
    p = i;

}

ArrayColorMap::ArrayColorMap(std::initializer_list<HSV> rhs)
    : ColorMap(RGB::Black), p(rhs.size()), size(rhs.size()) {
    entries = new RGB[rhs.size()];
    int i = 0;
    for (auto c : rhs) {
        entries[i++] = c;
    }
    p = i;
}

ArrayColorMap::ArrayColorMap(const ArrayColorMap& rhs)
    : p(rhs.p), size(rhs.size) {
    entries = new RGB[rhs.size];
    background = rhs.background;
    memcpy(entries, rhs.entries, sizeof(RGB) * rhs.size);
}

ArrayColorMap::~ArrayColorMap() { delete[] entries; }

RGB ArrayColorMap::operator[](uint8_t index) {
    if (index < 0 || index >= size) {
        return background;
    }
    return entries[index];
}

uint8_t ArrayColorMap::addColor(const RGB& color) {
    //
    // See if we already have this color.
    for (int i = 0; i < p; i++) {
        if (entries[i] == color) {
            return i;
        }
    }
    if (p >= size) return 0;
    uint8_t idx = p;
    entries[p++] = color;
    return idx;
}

uint8_t ArrayColorMap::addColor(const HSV& color) { return addColor(RGB(color)); }

uint8_t ArrayColorMap::addColors(std::initializer_list<RGB> colors) {
    uint8_t last = 0;
    for (auto c : colors) {
        last = addColor(c);
    }
    return last;
}

void ArrayColorMap::setColor(uint8_t index, const RGB& color) {
    if (index >= size) return;
    entries[index] = color;
}

void ArrayColorMap::setColor(uint8_t index, const HSV& color) {
    if (index >= size) return;
    entries[index] = RGB(color);
}

RGB ArrayColorMap::getColor(uint8_t index) {
    if (index == 255 || index >= size) return background;
    return entries[index];
}

uint8_t ArrayColorMap::getIndex(RGB& color) {
    for (int i = 0; i < p; i++) {
        if (entries[i] == color) {
            return i;
        }
    }
    return 0;
}
uint8_t ArrayColorMap::getIndex(HSV& color) {
    RGB r = RGB(color);
    return getIndex(r);
}

void ArrayColorMap::setBrightness(uint8_t value) {
    for (int i = 0; i < p; i++) {
        entries[i].nscale8_video(value);
    }
}
