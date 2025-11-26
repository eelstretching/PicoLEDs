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
  memset(&data[start], index, n * sizeof(uint8_t));
}

void Strip::rotateRight(int start, int end) {
  //
  // A place to put the data from the rightmost pixel.
  uint8_t tmp = data[end - 1];
  memmove(&data[start + 1], &data[start], (end - start - 1) * sizeof(uint8_t));
  data[start] = tmp;
}

void Strip::rotateLeft(int start, int end) {
  //
  // A place to put the data from the leftmost pixel.
  uint8_t tmp = data[start];
  memmove(&data[start], &data[start + 1], (end - start - 1) * sizeof(uint8_t));
  data[end - 1] = tmp;
}

void Strip::rotate(Direction direction, int n) {
  for (int i = 0; i < n; i++) {
    rotate(direction);
  }
}

void Strip::rotate(Direction direction) {
  uint8_t tmp;
  switch (direction) {
    case RIGHT:
    case UP:
      tmp = data[numPixels - 1];
      memcpy(&data[1], &data[0], (numPixels - 1) * sizeof(uint8_t));
      data[0] = tmp;
      break;
    case LEFT:
    case DOWN:
      tmp = data[0];
      memcpy(&data[0], &data[1], (numPixels - 1) * sizeof(uint8_t));
      data[numPixels - 1] = tmp;
      break;

  }
}
