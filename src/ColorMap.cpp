#include "ColorMap.h"

ColorMap::ColorMap() : ColorMap(16) {
    
}

ColorMap::ColorMap(uint8_t size) : size(size), p(0), background(RGB::Black) {
    entries = new RGB[size];
    for (uint8_t i = 0; i < size; i++) {
        entries[i] = RGB::Black;
    }
    p = 0;
}

ColorMap::ColorMap(std::initializer_list<RGB> rhs)
    : p(rhs.size()), size(rhs.size()), background(RGB::Black) {
    entries = new RGB[rhs.size()];
    int i = 0;
    for (auto c : rhs) {
        entries[i++] = c;
    }
    p = i;
}

ColorMap::ColorMap(std::initializer_list<HSV> rhs)
    : p(rhs.size()), size(rhs.size()), background(RGB::Black) {
    entries = new RGB[rhs.size()];
    int i = 0;
    for (auto c : rhs) {
        entries[i++] = c;
    }
    p = i;
}

ColorMap::ColorMap(std::initializer_list<uint32_t> rhs)
    : p(rhs.size()), size(rhs.size()), background(RGB::Black) {
    entries = new RGB[rhs.size()];
    int i = 0;
    for (auto c : rhs) {
        entries[i++] = c;
    }
}

ColorMap::ColorMap(const ColorMap& rhs)
    : p(rhs.p), size(rhs.size), background(RGB::Black) {
    entries = new RGB[size];
    for (uint8_t i = 0; i < size; i++) {
        entries[i] = rhs.entries[i];
    }
}

ColorMap::~ColorMap() { delete[] entries; }

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

void ColorMap::fillGradient(uint8_t startpos, const RGB& startcolor,
                            uint8_t endpos, const RGB& endcolor) {
    uint16_t rdistance87;
    uint16_t gdistance87;
    uint16_t bdistance87;

    rdistance87 = (endcolor.r - startcolor.r) << 7;
    gdistance87 = (endcolor.g - startcolor.g) << 7;
    bdistance87 = (endcolor.b - startcolor.b) << 7;

    uint16_t pixeldistance = endpos - startpos;
    int16_t divisor = pixeldistance ? pixeldistance : 1;

    uint16_t rdelta87 = rdistance87 / divisor;
    uint16_t gdelta87 = gdistance87 / divisor;
    uint16_t bdelta87 = bdistance87 / divisor;

    rdelta87 *= 2;
    gdelta87 *= 2;
    bdelta87 *= 2;

    uint16_t r88 = startcolor.r << 8;
    uint16_t g88 = startcolor.g << 8;
    uint16_t b88 = startcolor.b << 8;
    for (uint16_t i = startpos; i <= endpos; ++i) {
        entries[i] = RGB(r88 >> 8, g88 >> 8, b88 >> 8);
        r88 += rdelta87;
        g88 += gdelta87;
        b88 += bdelta87;
    }
}

RGB ColorMap::getColor(uint8_t index) {
    if (index == 255 || index >= size) return background;
    return entries[index];
}

uint8_t ColorMap::getIndex(RGB& color) {
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

void ColorMap::setBrightness(uint8_t value) {
    for (int i = 0; i < p; i++) {
        entries[i].nscale8_video(value);
    }
}
