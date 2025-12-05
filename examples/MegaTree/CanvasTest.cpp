#include <ColorBars.h>
#include <FadingBars.h>
#include <RotatingColumns.h>
#include <RotatingRows.h>
#include <TimedAnimation.h>
#include <stdlib.h>

#include "Animator.h"
#include "ArrayColorMap.h"
#include "BarberPole.h"
#include "Bouncer.h"
#include "Canvas.h"
#include "ColorCone.h"
#include "Icicles.h"
#include "LinesFill.h"
#include "Marquees.h"
#include "Spiral.h"
#include "Strip.h"
#include "colorutils.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/printf.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/types.h"

#define NUM_STRIPS 5
#define START_PIN 2
#define STRIP_LEN 400
#define CANVAS_WIDTH 100
#define BRIGHTNESS 16
#define FPS 30

int pico_led_init(void) {
#if defined(PICO_DEFAULT_LED_PIN)
    // A device like Pico that uses a GPIO for the LED will define
    // PICO_DEFAULT_LED_PIN so we can use normal GPIO functionality to turn the
    // led on and off
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    return PICO_OK;
#elif defined(CYW43_WL_GPIO_LED_PIN)
    // For Pico W devices we need to initialise the driver etc
    return cyw43_arch_init();
#endif
}

// Turn the led on or off
void pico_set_led(bool led_on) {
#if defined(PICO_DEFAULT_LED_PIN)
    // Just set the GPIO on or off
    gpio_put(PICO_DEFAULT_LED_PIN, led_on);
#elif defined(CYW43_WL_GPIO_LED_PIN)
    // Ask the wifi "driver" to set the GPIO on or off
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
#endif
}

int main() {
    stdio_init_all();

    int rc = pico_led_init();

    pico_set_led(true);
    sleep_ms(1000);
    pico_set_led(false);

    //
    // A canvas made out of strips.
    Strip* strips[NUM_STRIPS];
    int ns = NUM_STRIPS;
    int pin = START_PIN;
    int pins[NUM_STRIPS];
    for (int i = 0; i < ns; i++, pin++) {
        pins[i] = pin;
        //
        // First strip is a little short at the moment.
        strips[i] = new Strip(pin, STRIP_LEN);
        strips[i]->setColorOrder(ColorOrder::OGRB);
    }

    ArrayColorMap xmasColors({RGB::Red, RGB::Green, RGB::Blue, RGB::White,
                              RGB::Yellow, RGB::Orange});

    ArrayColorMap dimXmasColors(xmasColors);
    dimXmasColors.setBrightness(16);

    ArrayColorMap midXmasColors(xmasColors);
    midXmasColors.setBrightness(64);

    ArrayColorMap brightXmasColors(xmasColors);
    brightXmasColors.setBrightness(128);

    Canvas c(CANVAS_WIDTH);
    for (int i = 0; i < ns; i++) {
        c.add(*strips[i]);
    }
    c.setup();
    c.setColorMap(&dimXmasColors);

    for (int i = 0; i < c.getHeight(); i++) {
        c.fillRow(i, i % dimXmasColors.getUsed());
    }
    c.show();
    sleep_ms(10000);

    int nr = 0;
    while (true) {
        c.rotateUp();
        c.show();
        sleep_ms(250);
        nr++;
        if (nr % 10 == 0) {
            printf("Rotated %d times\n", nr);
        }
    }
}
