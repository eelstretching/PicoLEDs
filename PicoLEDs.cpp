/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>
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
  int t = 0;
  while (1) {
    int dir = (rand() >> 30) & 1 ? 1 : -1;
    for (int i = 0; i < 1000; ++i) {
      pattern_snakes(strip, t);
      sleep_ms(10);
      t += dir;
    }
  }
}
