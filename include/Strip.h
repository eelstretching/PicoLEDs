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


class Strip {
   protected:
    //
    // The pin this strip is on.
    uint pin;

    //
    // The bytes making up the data for this strip.
    RGB *data;

    //
    // How many pixels there are in the strip. For now, we're going to assume
    // that strips are RGB.
    uint numPixels;

    //
    // The color order for this strip. Default is RGB.
    ColorOrder colorOrder = ColorOrder::ORGB;

    //
    // We'll keep things dim by default.
    uint8_t fracBrightness = 0x20;

    //
    // Where the next pixel will be added.
    uint pos;

    Strip() {};

   public:

    Strip(uint pin, uint num_pixels);

    /// @brief Adds a pixel to this strip at the next position
    /// @param c The RGB color value to add for the pixel
    /// @return The position that the pixel was added.
    uint addPixel(RGB c);

    /// @brief Adds a pixel to this strip at the next position
    /// @param h The HSV color value to add for the pixel
    /// @return The position that the pixel was added.
    uint addPixel(HSV c);

    /// @brief Puts a color at a specific pixel in the strip.
    /// @param p The position where the pixel should be placed. If this is not
    /// within the bounds of the array, then no change will happen
    /// @param c The RGB color value to put in the strip.
    void putPixel(RGB c, uint p);

    /// @brief Gets the color value at a given position in the strip
    /// @param p the position to get the color from
    /// @return the color at the given position.
    RGB get(uint p);

    /// @brief Puts n pixels from the given array into the strip, starting at
    /// position 0.
    /// @param pixels The data that we want to copy into our strip
    /// @param n The number of pixels to copy.
    void putPixels(RGB *pixels, uint n);

    /// @brief Puts n pixels from the given array into the strip, starting at
    /// position p.
    /// @param p The position in the strip where pixels should be copied.
    /// @param pixels The data that we want to copy into our strip
    /// @param n The number of pixels to copy.
    void putPixels(RGB *pixels, uint p, uint n);

    /// @brief Fills the strip with the given color.
    /// @param c The color to fill the strip with.
    void fill(RGB c);

    /// @brief Fills a range of the strip with a given color
    /// @param c The color to fill with
    /// @param start the index where to start
    /// @param n the number of pixels to fill
    void fill(RGB c, uint start, uint n);

    uint getNumPixels() { return numPixels; }

    /// @brief Sets the fractional brightness for the whole strip.
    /// @param fractionalBrightness A value between 0 and 255
    /// that specifies the brightness level of the LEDs in the strip between 0
    /// (off) and 255 (all the way on).
    void setFractionalBrightness(uint8_t fractionalBrightness) {
        fracBrightness = fractionalBrightness;
    }

    int getPin() { return pin;}

    ColorOrder getColorOrder() { return colorOrder; }

    void setColorOrder(ColorOrder co) { colorOrder = co; }

    RGB *getData() { return data; }

    uint8_t getFractionalBrightness() { return fracBrightness; }

    void reset() { pos = 0; }

};

#endif