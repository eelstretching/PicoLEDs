#ifndef STRIP_H
#define STRIP_H

#pragma once

#include "color.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include "pico/types.h"

class Strip {
  //
  // The pin this strip is on.
  uint pin;

  //
  // The PIO unit that it's using.
  PIO pio;

  //
  // The state machine in the PIO that we're using.
  int sm;

  //
  // The offset of the PIO program to run the strip.
  int offset;

  //
  // The bytes making up the data for this strip.
  RGB *data;

  //
  // The color order for this strip. Default is GRB.
  ColorOrder color_order = ColorOrder::ORGB;

  //
  // How many pixels there are in the strip. For now, we're going to assume that
  // strips are RGB.
  uint num_pixels;

  //
  // We'll keep things dim by default.
  uint8_t fracBrightness = 0x20;

  //
  // Where the next pixel will be added.
  uint pos;

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
  /// @param p The position where the pixel should be placed. If this is not within the bounds of the array, then no change will happen
  /// @param c The RGB color value to put in the strip.
  void putPixel(uint p, RGB c);

  /// @brief Puts n pixels from the given array into the strip, starting at position 0.
  /// @param pixels The data that we want to copy into our strip
  /// @param n The number of pixels to copy.
  void putPixels(RGB *pixels, uint n);

  /// @brief Puts n pixels from the given array into the strip, starting at position p.
  /// @param p The position in the strip where pixels should be copied.
  /// @param pixels The data that we want to copy into our strip
  /// @param n The number of pixels to copy.
  void putPixels(uint p, RGB *pixels, uint n);

  /// @brief Fills the strip with the given color.
  /// @param c The color to fill the strip with.
  void fill(RGB c);

  /// @brief Shows the strip, i.e., it sends the data out the PIO state machine.
  void show();

  uint getNumPixels() { return num_pixels; }

  ColorOrder getColorOrder() {return color_order;}

  void setColorOrder(ColorOrder co) {color_order = co;}

  /// @brief Sets the fractional brightness for the whole strip. 
  /// @param fractionalBrightness A value between 0 and 255 
  /// that specifies the brightness level of the LEDs in the strip between 0 (off) and 255 (all the way on).
  void setFractionalBrightness(uint8_t fractionalBrightness) {
    fracBrightness = fractionalBrightness;
  }

  uint8_t getFractionalBrightness() {
    return fracBrightness;
  }

  void reset() {
    pos = 0;
  }
};

#endif