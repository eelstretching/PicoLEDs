#include <Renderer.h>
#include <stdlib.h>

#include "ArrayColorMap.h"
#include "StopWatch.h"
#include "Panel.h"
#include "colorutils.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/printf.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/types.h"

#define STRIP_LEN 256
#define START_PIN 2
#define NUM_PANELS 16
#define BAND_WIDTH 8
#define PANEL_WIDTH 32
#define PANEL_HEIGHT 8

#define START_FILL_PANEL 0

int main() {
    stdio_init_all();

    //
    // Simple test for a few strips of pixels.
    Panel* panels[NUM_PANELS];
    Renderer renderer(8);
    int ns = NUM_PANELS;
    int pin = START_PIN;
    for (int i = 0; i < ns; i++) {
        panels[i] = new Panel(pin++, PANEL_WIDTH, PANEL_HEIGHT);
        renderer.add(panels[i]);
    }
    renderer.setup();
    renderer.setBrightness(8);
    for (int i = 0; i < ns; i++) {
        panels[i]->fill(RGB::Black);
    }
    renderer.render();
    sleep_ms(10000);
}
