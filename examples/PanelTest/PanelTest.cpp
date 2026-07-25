#include <Renderer.h>
#include <stdlib.h>

#include "ArrayColorMap.h"
#include "Panel.h"
#include "PanelCanvas.h"
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
#define WIDTH 32
#define HEIGHT 8

int main() {
    stdio_init_all();

    sleep_ms(2000);

    ArrayColorMap colorMap({RGB::Red, RGB::Orange, RGB::Yellow, RGB::Green,
                            RGB::Blue, RGB::Indigo, RGB::Violet, RGB::White,
                            RGB::Gold});

    Panel panels[6] = {Panel(START_PIN, WIDTH, HEIGHT),
                       Panel(START_PIN + 1, WIDTH, HEIGHT),
                       Panel(START_PIN + 2, WIDTH, HEIGHT),
                       Panel(START_PIN + 3, WIDTH, HEIGHT),
                       Panel(START_PIN + 4, WIDTH, HEIGHT),
                       Panel(START_PIN + 5, WIDTH, HEIGHT)};

  
    PanelCanvas canvas(WIDTH, HEIGHT, 3, 2);
    canvas.addPanel(&panels[0], 0, 0);
    canvas.addPanel(&panels[1], 1, 0);
    canvas.addPanel(&panels[2], 2, 0);
    canvas.addPanel(&panels[3], 0, 1);
    canvas.addPanel(&panels[4], 1, 1);
    canvas.addPanel(&panels[5], 2, 1);
    canvas.setup();
    canvas.setBrightness(32);

    for (int i = 0; i < colorMap.getUsed(); i++) {
        printf("Filling with color %d\n", i);
        canvas.fill(colorMap.getColor(i));
        canvas.show();
        sleep_ms(250);
    }

    canvas.fill(colorMap.getBackground());
    canvas.show();
    sleep_ms(100);

    int c[] = {0, 1, 2};
    while (1) {
        for (int i = 0; i < canvas.getWidth() - canvas.getHeight(); i++) {
            for (int j = 0; j < canvas.getHeight(); j++) {
                canvas.fill(colorMap.getBackground());
                int x = i + j;
                canvas.set(x, j, colorMap.getColor(c[2]));
                if (x > 1) {
                    canvas.set(x - 1, j, colorMap.getColor(c[1]));
                }
                if (x > 2) {
                    canvas.set(x - 2, j, colorMap.getColor(c[0]));
                }
                canvas.show();
                sleep_ms(50);
            }
        }
        c[0] = (c[0] + 1) % colorMap.getUsed();
        c[1] = (c[1] + 1) % colorMap.getUsed();
        c[2] = (c[2] + 1) % colorMap.getUsed();
    }
}
