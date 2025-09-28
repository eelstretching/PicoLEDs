#ifndef COLORMAP_H
#define COLORMAP_H

#pragma once

#include <initializer_list>

#include "color.h"
#include "pico/stdlib.h"
#include "pico/types.h"

/// @brief A color map that maps from 8-bit values to RGB colors.
/// By construction we'll use the 0th entry in the color map as the background
/// for whatever canvas we're drawing on.
/// Color maps are a fixed size, but can be added to as they go.
class ColorMap {
   protected:
    uint8_t size;
    uint8_t p;
    RGB* entries;

   public:
    ColorMap(uint8_t size);
    ColorMap(std::initializer_list<RGB> rhs);
    ColorMap(std::initializer_list<HSV> rhs);
    ColorMap(std::initializer_list<uint32_t> rhs);
    ColorMap(const ColorMap& rhs);
    ~ColorMap();

    uint8_t getSize() { return size; };
    uint8_t getUsed() { return p; };

    RGB operator[](uint8_t index);
    uint8_t addColor(const RGB& color);
    uint8_t addColor(const HSV& color);
    uint8_t addColors(std::initializer_list<RGB> colors);
    void setColor(uint8_t index, const RGB& color);
    void setColor(uint8_t index, const HSV& color);
    RGB getColor(uint8_t index);
    uint8_t getIndex(RGB& color);
    uint8_t getIndex(HSV& color);
    void dim(uint8_t value);
};


#endif
