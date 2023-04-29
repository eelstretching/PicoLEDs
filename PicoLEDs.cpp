/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "Strip.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/printf.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/types.h"
#include "ws2812.pio.h"

void pattern_snakes(Strip strip, uint t) {
  strip.reset();
  for (uint i = 0; i < strip.getNumPixels(); ++i) {
    uint x = (i + (t >> 1)) % 64;
    if (x < 10) {
      strip.addPixel(RGB::Red);
    } else if (x >= 15 && x < 25) {
      strip.addPixel(RGB::Green);
    } else if (x >= 30 && x < 40) {
      strip.addPixel(RGB::Blue);
    } else {
      strip.addPixel(RGB::Black);
    }
  }
  strip.show();
}

int main() {
  stdio_init_all();

  Strip strip(3, 32);
  printf("Started strip with having %d LEDs\n", strip.getNumPixels());
  while (1) {
    strip.fill(RGB::Black);
    strip.show();
    sleep_ms(500);
    for (int i = 0; i < strip.getNumPixels(); i++) {
      switch (i % 3) {
        case 0:
          strip.addPixel(RGB::Red);
          break;
        case 1:
          strip.addPixel(RGB::Green);
          break;
        case 2:
          strip.addPixel(RGB::Blue);
          break;
      }
    }
    strip.show();
    sleep_ms(2000);
  }
}
