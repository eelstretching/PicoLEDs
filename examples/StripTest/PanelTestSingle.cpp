#include <Renderer.h>
#include <stdlib.h>

#include "ArrayColorMap.h"
#include "Panel.h"
#include "StopWatch.h"
#include "colorutils.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/printf.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/types.h"

#define STRIP_LEN 256
#define NUM_STRIPS 1
#define START_PIN 2
#define PANEL_WIDTH 32
#define PANEL_HEIGHT 8

int main() {
    stdio_init_all();

    sleep_ms(2000);

    //
    // Simple test for a single strip of pixels.
    Panel panel(START_PIN, PANEL_WIDTH, PANEL_HEIGHT);
    Renderer renderer(16);
    renderer.add(&panel);
    renderer.setup();

    ArrayColorMap colorMap({RGB::Red, RGB::Orange, RGB::Yellow, RGB::Green,
                            RGB::Blue, RGB::Indigo, RGB::Violet, RGB::White,
                            RGB::Gold});
    for (int i = 0; i < colorMap.getUsed(); i++) {
        printf("Filling with color %d\n", i);
        panel.fill(colorMap.getColor(i));
        renderer.render();
        sleep_ms(250);
    }

    panel.fill(colorMap.getBackground());
    renderer.render();
    sleep_ms(100);

    for (int i = 0; i < panel.getNumPixels(); i++) {
        panel.fill(colorMap.getBackground());
        panel.putPixel(RGB::Red, i);
        if (i >= 1) {
            panel.putPixel(RGB::Green, i - 1);
        }
        if (i >= 2) {
            panel.putPixel(RGB::Blue, i - 2);
        }
        renderer.render();
        sleep_ms(50);
    }

    int c[] = {0, 1, 2};
    while (1) {
        for (int i = 0; i < panel.getWidth() - panel.getHeight(); i++) {
            for (int j = 0; j < panel.getHeight(); j++) {
                panel.fill(colorMap.getBackground());
                int x = i + j;
                panel.set(x, j, colorMap.getColor(c[2]));
                if (x > 1) {
                    panel.set(x - 1, j, colorMap.getColor(c[1]));
                }
                if (x > 2) {
                    panel.set(x - 2, j, colorMap.getColor(c[0]));
                }
                renderer.render();
                sleep_ms(50);
            }
        }
        c[0] = (c[0] + 1) % colorMap.getUsed();
        c[1] = (c[1] + 1) % colorMap.getUsed();
        c[2] = (c[2] + 1) % colorMap.getUsed();
    }
}
