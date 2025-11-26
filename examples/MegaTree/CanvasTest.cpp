#include <Canvas.h>
#include <Renderer.h>
#include <stdlib.h>

#include "ArrayColorMap.h"
#include "FadeColorMap.h"
#include "StopWatch.h"
#include "Strip.h"
#include "colorutils.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/printf.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/types.h"

#define NUM_STRIPS 1
#define START_PIN 2
#define STRIP_LEN 300
#define CANVAS_WIDTH 100
#define BRIGHTNESS 32
#define FPS 30

int main() {
    stdio_init_all();

    //
    // A canvas made out of strips.
    Strip* strips[NUM_STRIPS];
    int ns = NUM_STRIPS;
    int pin = START_PIN;
    int pins[NUM_STRIPS];
    for (int i = 0; i < ns; i++) {
        pins[i] = pin;
        strips[i] = new Strip(pin++, STRIP_LEN);
        strips[i]->setColorOrder(ColorOrder::ORGB);
    }

    ArrayColorMap colorMap({RGB::Red, RGB::Green, RGB::Blue, RGB::White,
                            RGB::Yellow, RGB::Orange});

    colorMap.setBrightness(BRIGHTNESS);

    Canvas c(CANVAS_WIDTH);

    for (int i = 0; i < ns; i++) {
        c.add(*strips[i]);
    }
    c.setup();
    c.setColorMap(&colorMap);

    c.fillRow(0, 0);
    c.fillRow(1, 1);
    c.fillRow(2, 2);
    c.show();
    printf("Row 0: %d %d Row 1: %d %d Row 2: %d %d\n", c.getRow(0)->get(0), c.getRow(0)->get(7), c.getRow(1)->get(0), c.getRow(1)->get(7), c.getRow(2)->get(0), c.getRow(2)->get(7));
    sleep_ms(2000);
    printf("Row 0: %d %d Row 1: %d %d Row 2: %d %d\n", c.getRow(0)->get(0), c.getRow(0)->get(7), c.getRow(1)->get(0), c.getRow(1)->get(7), c.getRow(2)->get(0), c.getRow(2)->get(7));

    while (true) {
        c.rotateUp();
        c.show();
        sleep_ms(2000);
    }
}
