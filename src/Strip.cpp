#include "Strip.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/printf.h"


Strip::Strip(uint pin, uint numPixels, StripType type) : pin(pin), numPixels(numPixels), type(type) {
  data = new RGB[numPixels];
  pos = 0;
}

uint Strip::addPixel(const RGB& color) {
  uint p = pos;
  data[pos++] = color;
  return p;
}

void Strip::putPixel(const RGB& color, uint p) {
  if (p >= numPixels) {
    return;
  }
  data[p] = color;
}

const RGB& Strip::get(uint p) {
  if (p >= numPixels) {
    return stripBlack;
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

void Strip::fill(const RGB& color) { fill(color, 0, numPixels); }

void Strip::fill(const RGB& color, uint start, uint n) {
  if(start+n > numPixels) {
    n = numPixels - start;
  }
  RGB *dp = &data[start];
  for(int i = 0; i < n; i++) {
    *dp++ = color;
  }
}

void Strip::rotateRight(int start, int end) {
  //
  // A place to put the data from the rightmost pixel.
  RGB tmp = data[end - 1];
  memmove(&data[start + 1], &data[start], (end - start - 1) * sizeof(RGB));
  data[start] = tmp;
}

void Strip::rotateLeft(int start, int end) {
  //
  // A place to put the data from the leftmost pixel.
  RGB tmp = data[start];
  memmove(&data[start], &data[start + 1], (end - start - 1) * sizeof(RGB));
  data[end - 1] = tmp;
}

void Strip::rotate(Direction direction, int n) {
  for (int i = 0; i < n; i++) {
    rotate(direction);
  }
}

void Strip::rotate(Direction direction) {
  RGB tmp;
  switch (direction) {
    case RIGHT:
    case UP:
      tmp = data[numPixels - 1];
      memcpy(&data[1], &data[0], (numPixels - 1) * sizeof(RGB));
      data[0] = tmp;
      break;
    case LEFT:
    case DOWN:
      tmp = data[0];
      memcpy(&data[0], &data[1], (numPixels - 1) * sizeof(RGB));
      data[numPixels - 1] = tmp;
      break;

  }
}
