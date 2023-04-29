#include "Strip.h"

#include "ws2812.pio.h"

#include <string.h>

Strip::Strip(uint pin, uint num_pixels) : pin(pin), num_pixels(num_pixels) {
  data = new RGB[num_pixels];
  pos = 0;

  // find an unclaimed PIO state machine and put the program there. Looks like
  // this will panic the machine if we try to add too many?
  const PIO pios[NUM_PIOS] = {pio0, pio1};
  // iterate over PIO instances
  for (unsigned int i = 0; i < NUM_PIOS; i++) {
    pio = pios[i];
    sm = pio_claim_unused_sm(pio, false);  // claim a state machine
    if (sm == -1) {
      continue;  // skip this PIO if no unused sm
    }
    offset = pio_add_program(pio, &ws2812_program);
    if (offset == -1) {
      pio_sm_unclaim(pio, sm);  // unclaim the state machine and skip this PIO
      continue;                 // if program couldn't be added
    }

    break;  // found pio and sm that work
  }
  if (offset == -1) return;  // couldn't find good pio and sm
  ws2812_program_init(pio, sm, offset, pin, 800000);
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

void Strip::putPixel(uint p, RGB c) {
  if(p >= num_pixels) {
    return;
  }
  data[p] = c;
}

void Strip::putPixels(RGB* pixels, uint n) {
  putPixels(0, pixels, n);
}

void Strip::putPixels(uint p, RGB* pixels, uint n) {
  if(p >= num_pixels) {
    return;
  }
  if(p + n >= num_pixels) {
    n = num_pixels - p;
  }
  memcpy(&data[p], pixels, n * sizeof(RGB));

}

void Strip::fill(RGB c) {
  for(int i = 0; i < num_pixels; i++) {
    data[i] = c;
  }
}

void Strip::show() {
    for(int i = 0; i < num_pixels; i++) {
        pio_sm_put_blocking(pio, sm, data[i].scale8(fracBrightness).getColor(color_order) << 8u);
    }
    pos = 0;
}