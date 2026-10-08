// Desktop simulator stand-in for the Pico SDK's hardware/pio.h. Just enough
// for Renderer.h to compile; the simulated Renderer doesn't use PIO.
#ifndef PICOLEDS_SIM_HARDWARE_PIO_H
#define PICOLEDS_SIM_HARDWARE_PIO_H

#include "hardware/dma.h"
#include "pico/types.h"

typedef struct pio_hw pio_hw_t;
typedef pio_hw_t* PIO;

typedef struct {
    const uint16_t* instructions;
    uint8_t length;
    int8_t origin;
} pio_program_t;

#endif
