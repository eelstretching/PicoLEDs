#ifndef STRIP_H
#define STRIP_H

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
  uint32_t *data;

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
  /// @param r 0-255 red
  /// @param g 0-255 green
  /// @param b 0-255 blude
  /// @return The position that the pixel was added.
  uint addPixel(uint8_t r, uint8_t g, uint8_t b);

  /// @brief Shows the strip, i.e., it sends the data out the PIO state machine.
  void show();

  uint getNumPixels() { return num_pixels; }

  void setFractionalBrightness(uint fractionalBrightness) {
    fracBrightness = fractionalBrightness;
  }
};

#endif