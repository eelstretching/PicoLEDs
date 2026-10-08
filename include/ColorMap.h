#ifndef COLORMAP_H
#define COLORMAP_H

#pragma once

#include <initializer_list>

#include "color.h"
#include "colorutils.h"
#include "pico/stdlib.h"
#include "pico/types.h"

/// @brief A color map that maps from 8-bit values to RGB colors.
/// We also have a color to use as the background, since that's kind of a separate thing. 
/// This color map is mostly abstract; specific implementations will derive from this class.
class ColorMap {
   protected:
    RGB background = RGB::Black;

   public:
    ColorMap() {}
    ColorMap(RGB background) : background(background) {};
    ~ColorMap() {};

    virtual uint8_t getSize() { return 0; };
    virtual uint8_t getUsed() { return 0; };

    void setBackground(const RGB& color) { background = color; };
    const RGB& getBackground() { return background; };
    uint8_t getBackgroundIndex() {return 255;};
    uint8_t getRandomColor() {return random8(getUsed());};
    virtual RGB operator[](uint8_t index) { return background; };
    virtual uint8_t addColor(const RGB& color) {return 0;};
    virtual uint8_t addColor(const HSV& color) {return 0;};
    virtual void setColor(uint8_t index, const RGB& color) {};
    virtual void setColor(uint8_t index, const HSV& color) {};
    virtual uint8_t addColors(std::initializer_list<RGB> colors) {return 0;};
    virtual RGB getColor(uint8_t index) {return RGB::Black;};
    virtual uint8_t getIndex(RGB& color) { return 0;};
    virtual uint8_t getIndex(HSV& color) { return 0;};
    virtual void setBrightness(uint8_t value) {};

    /// @brief Fills a 16-entry palette with the colors in this map, spread
    /// evenly across the palette in order. A map with red, green and white
    /// gives a palette that's about a third each red, green and white. Read
    /// with ColorFromPalette and NOBLEND, that picks one of the map's colors;
    /// with LINEARBLEND, it slides smoothly from one color to the next.
    /// @param pal the palette to fill
    void toPalette(RGBPalette16& pal) {
        uint8_t n = getUsed();
        for (int k = 0; k < 16; k++) {
            pal.entries[k] = n == 0 ? background : getColor((k * n) / 16);
        }
    }
};


#endif
