#include "Strip.h"

#include "ws2812.pio.h"

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (g) << 8) |
            ((uint32_t) (r) << 16) |
            (uint32_t) (b);
}

Strip::Strip(uint pin, uint num_pixels) : pin(pin), num_pixels(num_pixels) {
  data = new uint32_t[num_pixels];
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

uint Strip::addPixel(uint8_t r, uint8_t g, uint8_t b) {
    uint p = pos;
    data[pos++] = urgb_u32(r, g, b);
    return p;
}

void Strip::show() {
    for(int i = 0; i < num_pixels; i++) {
        pio_sm_put_blocking(pio, sm, (data[i] * fracBrightness) >> 8u);
    }
}