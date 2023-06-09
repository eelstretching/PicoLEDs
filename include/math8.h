#ifndef MATH8_H
#define MATH8_H

#pragma once

#include "pico/stdlib.h"
#include "pico/types.h"

typedef uint8_t fract8;

static inline uint8_t qadd8(uint8_t i, uint8_t j) {
  unsigned int t = i + j;
  return t > 255 ? 255 : t;
}

static inline uint8_t qsub8(uint8_t i, uint8_t j) {
  int t = i - j;
  return t < 0 ? 0 : t;
}

static inline uint8_t avg8(uint8_t i, uint8_t j) { return (i + j) >> 1; }

static inline uint8_t mod8(uint8_t a, uint8_t m) {
  while (a >= m) a -= m;
  return a;
}

static inline uint8_t mul8(uint8_t i, uint8_t j) { return ((int)i * (int)(j)) & 0xFF; }

static inline uint8_t qmul8(uint8_t i, uint8_t j) {
  unsigned p = (unsigned)i * (unsigned)j;
  return p > 255 ? 255 : p;
}

static inline void nscale8x3(uint8_t& r, uint8_t& g, uint8_t& b, fract8 scale) {
  uint16_t scale_fixed = scale + 1;
  r = (((uint16_t)r) * scale_fixed) >> 8;
  g = (((uint16_t)g) * scale_fixed) >> 8;
  b = (((uint16_t)b) * scale_fixed) >> 8;
}

static inline void nscale8x3_video(uint8_t& r, uint8_t& g, uint8_t& b, fract8 scale) {
  uint8_t nonzeroscale = (scale != 0) ? 1 : 0;
  r = (r == 0) ? 0 : (((int)r * (int)(scale)) >> 8) + nonzeroscale;
  g = (g == 0) ? 0 : (((int)g * (int)(scale)) >> 8) + nonzeroscale;
  b = (b == 0) ? 0 : (((int)b * (int)(scale)) >> 8) + nonzeroscale;
}

static inline uint8_t scale8(uint8_t i, fract8 scale) {
  return (((uint16_t)i) * (1 + (uint16_t)(scale))) >> 8;
}

static inline int8_t scale8_LEAVING_R1_DIRTY(uint8_t i, fract8 scale) {
  return (((uint16_t)i) * ((uint16_t)(scale) + 1)) >> 8;
}

static inline uint8_t scale8_video(uint8_t i, fract8 scale) {
  return (((int)i * (int)scale) >> 8) + ((i && scale) ? 1 : 0);
}

static inline uint8_t lerp8by8(uint8_t a, uint8_t b, fract8 frac) {
  uint8_t result;
  if (b > a) {
    uint8_t delta = b - a;
    uint8_t scaled = scale8(delta, frac);
    result = a + scaled;
  } else {
    uint8_t delta = a - b;
    uint8_t scaled = scale8(delta, frac);
    result = a - scaled;
  }
  return result;
}

#endif