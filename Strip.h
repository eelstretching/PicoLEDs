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
  ColorOrder color_order = ColorOrder::OGRB;

  //
  // How many pixels there are in the strip. For now, we're going to assume that
  // strips are RGB.
  uint num_pixels;

  //
  // We'll keep things dim by default.
  uint fracBrightness = 0x20;

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

  /// @brief Shows the strip, i.e., it sends the data out the PIO state machine.
  void show();

  uint getNumPixels() { return num_pixels; }

  void setFractionalBrightness(uint fractionalBrightness) {
    fracBrightness = fractionalBrightness;
  }
};

#endif