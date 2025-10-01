#include "ColorMap.h"

ColorMap::ColorMap(uint8_t size) : size(size), p(0) {
    entries = new RGB[size];
    for (uint8_t i = 0; i < size; i++) {
        entries[i] = RGB::Black;
    }
    p = 0;
}

ColorMap::ColorMap(std::initializer_list<RGB> rhs) : p(rhs.size()), size(rhs.size()) {
    entries = new RGB[rhs.size()];
    int i = 0;
    for (auto c : rhs) {
        entries[i++] = c;
    }
    p = i;
}

ColorMap::ColorMap(std::initializer_list<HSV> rhs)  : p(rhs.size()), size(rhs.size()) {
    entries = new RGB[rhs.size()];
    int i = 0;
    for (auto c : rhs) {
        entries[i++] = c;
    }
    p = i;
}

ColorMap::ColorMap(std::initializer_list<uint32_t> rhs) : p(rhs.size()), size(rhs.size()) {
    entries = new RGB[rhs.size()];
    int i = 0;
    for (auto c : rhs) {
        entries[i++] = c;
    }
}

ColorMap::ColorMap(const ColorMap& rhs) {
    size = rhs.size;
    p = rhs.p;
    entries = new RGB[size];
    for (uint8_t i = 0; i < size; i++) {
        entries[i] = rhs.entries[i];
    }
}

ColorMap::~ColorMap() {
    delete[] entries;
}

RGB ColorMap::operator[](uint8_t index) {
    if (index < 0 || index >= size) {
        return background;
    }
    return entries[index];
}

uint8_t ColorMap::addColor(const RGB& color) {
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

uint8_t ColorMap::addColor(const HSV& color) { return addColor(RGB(color)); }

uint8_t ColorMap::addColors(std::initializer_list<RGB> colors) { 
    uint8_t last = 0;
    for (auto c : colors) {
        last = addColor(c);
    }
    return last;
}
 

void ColorMap::setColor(uint8_t index, const RGB& color) {
    if (index >= size) return;
    entries[index] = color;
}

void ColorMap::setColor(uint8_t index, const HSV& color) {
    if (index >= size) return;
    entries[index] = RGB(color);
}

RGB ColorMap::getColor(uint8_t index) {
    if(index == 255 || index >= size) return background;
    return entries[index];
}

uint8_t ColorMap::getIndex(RGB &color) {
    for (int i = 0; i < p; i++) {
        if (entries[i] == color) {
            return i;
        }
    }
    return 0;
}
uint8_t ColorMap::getIndex(HSV& color) { 
    RGB r = RGB(color);
    return getIndex(r); 
}

void ColorMap::dim(uint8_t value) {
    for (int i = 0; i < p; i++) {
        entries[i].fadeToBlackBy(value);
    }
}
