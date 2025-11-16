#ifndef STRIP_H
#define STRIP_H

#pragma once

#include "StopWatch.h"
#include "color.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/sem.h"
#include "pico/stdlib.h"
#include "pico/types.h"
#include "ColorMap.h"
#include "Direction.h"

//
// @brief The type of strip we're dealing with.
enum StripType {
    WS2812,
    WS2811
};

class Strip {
   protected:
    //
    // The pin this strip is on.
    uint pin;

    //
    // The color map we'll use for rendering this strip.
    ColorMap *colorMap;
    
    //
    // The bytes making up the data for this strip. These are indices into the color map for this strip.
    uint8_t *data;

    //
    // How many pixels there are in the strip. For now, we're going to assume
    // that strips are RGB.
    uint numPixels;

    //
    // The color order for this strip. Default is GRB for WS2812B, per the data sheet.
    ColorOrder colorOrder = ColorOrder::OGRB;

    //
    // Where the next pixel will be added.
    uint pos;

    StripType type;

    Strip() {};

   public:

    Strip(uint pin, uint num_pixels, StripType type = WS2811);

    /// @brief Gets the type of strip this is.
    StripType getType() { return type; }

    /// @brief Adds a pixel to this strip at the next position
    /// @param colorIndex the index into the color map for the pixel
    /// @return The position that the pixel was added.
    uint addPixel(uint8_t colorIndex);

    /// @brief Puts a color at a specific pixel in the strip.
    /// @param p The position where the pixel should be placed. If this is not
    /// within the bounds of the array, then no change will happen
    /// @param colorIndex The index into the color map for the pixel.
    void putPixel(uint8_t colorIndex, uint p);

    /// @brief Gets the index of the color value at a given position in the strip
    /// @param p the position to get the color from
    /// @return the color at the given position.
    uint8_t get(uint p);

    /// @brief Puts n pixels from the given array into the strip, starting at
    /// position 0.
    /// @param pixels The data that we want to copy into our strip
    /// @param n The number of pixels to copy.
    void putPixels(uint8_t *pixels, uint n);

    /// @brief Puts n pixels from the given array into the strip, starting at
    /// position p.
    /// @param p The position in the strip where pixels should be copied.
    /// @param pixels The data that we want to copy into our strip
    /// @param n The number of pixels to copy.
    void putPixels(uint8_t *pixels, uint p, uint n);

    /// @brief Fills the strip with the given color.
    /// @param index The index into the color map for the color to fill with.
    void fill(uint8_t index);

    /// @brief Fills a range of the strip with a given color
    /// @param colorIndex the index into the color map for the color to fill with
    /// @param start the index where to start
    /// @param n the number of pixels to fill
    void fill(uint8_t colorIndex, uint start, uint n);

    void rotate(Direction direction, int n);

    void rotate(Direction direction);

    uint getNumPixels() { return numPixels; }

    int getPin() { return pin;}

    ColorOrder getColorOrder() { return colorOrder; }

    void setColorOrder(ColorOrder co) { colorOrder = co; }

    uint8_t *getData() { return data; }

    ColorMap *getColorMap() { return colorMap; }    

    void reset() { pos = 0; }

};

#endif