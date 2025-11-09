#include "Strip.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/printf.h"


Strip::Strip(uint pin, uint numPixels, StripType type) : pin(pin), numPixels(numPixels), type(type) {
  data = new uint8_t[numPixels];
  pos = 0;
}

uint Strip::addPixel(uint8_t index) {
  uint p = pos;
  data[pos++] = index;
  return p;
}

void Strip::putPixel(uint8_t index, uint p) {
  if (p >= numPixels) {
    return;
  }
  data[p] = index;
}

uint8_t Strip::get(uint p) {
  if (p >= numPixels) {
    return 0;
  }
  return data[p];
}

void Strip::putPixels(uint8_t* pixels, uint n) { putPixels(pixels, 0, n); }

void Strip::putPixels(uint8_t* pixels, uint p, uint n) {
  if (p >= numPixels) {
    return;
  }
  if (p + n >= numPixels) {
    n = numPixels - p;
  }
  memcpy(&data[p], pixels, n * sizeof(uint8_t));
}

void Strip::fill(uint8_t index) { fill(index, 0, numPixels); }

void Strip::fill(uint8_t index, uint start, uint n) {
  for (int i = start; i < start + n && i < numPixels; i++) {
    data[i] = index;
  }
}
