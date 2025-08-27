#include "Strip.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/printf.h"


Strip::Strip(uint pin, uint numPixels) : pin(pin), numPixels(numPixels) {
  data = new RGB[numPixels];
  pos = 0;
}

uint Strip::addPixel(RGB c) {
  uint p = pos;
  data[pos++] = c;
  return p;
}

uint Strip::addPixel(HSV c) {
  uint p = pos;
  hsv2rgb_raw(c, data[pos++]);
  return p;
}

void Strip::putPixel(RGB c, uint p) {
  if (p >= numPixels) {
    return;
  }
  data[p] = c;
}

RGB Strip::get(uint p) {
  if (p >= numPixels) {
    return RGB::Black;
  }
  return data[p];
}

void Strip::putPixels(RGB* pixels, uint n) { putPixels(pixels, 0, n); }

void Strip::putPixels(RGB* pixels, uint p, uint n) {
  if (p >= numPixels) {
    return;
  }
  if (p + n >= numPixels) {
    n = numPixels - p;
  }
  memcpy(&data[p], pixels, n * sizeof(RGB));
}

void Strip::fill(RGB c) { fill(c, 0, numPixels); }

void Strip::fill(RGB c, uint start, uint n) {
  for (int i = start; i < start + n && i < numPixels; i++) {
    data[i] = c;
  }
}
