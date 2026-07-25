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
#define PANEL_WIDTH 32
#define PANEL_HEIGHT 8
#define BAR_WIDTH 5

int main() {
    stdio_init_all();

    ArrayColorMap colorMap({RGB::Red, RGB::Orange, RGB::Yellow, RGB::Green,
                            RGB::Blue, RGB::Indigo, RGB::Violet, RGB::White,
                            RGB::Gold});

    Panel panels[6] = {Panel(START_PIN, PANEL_WIDTH, PANEL_HEIGHT),
                       Panel(START_PIN + 1, PANEL_WIDTH, PANEL_HEIGHT),
                       Panel(START_PIN + 2, PANEL_WIDTH, PANEL_HEIGHT),
                       Panel(START_PIN + 3, PANEL_WIDTH, PANEL_HEIGHT),
                       Panel(START_PIN + 4, PANEL_WIDTH, PANEL_HEIGHT),
                       Panel(START_PIN + 5, PANEL_WIDTH, PANEL_HEIGHT)};

  
    PanelCanvas canvas(PANEL_WIDTH, PANEL_HEIGHT, 3, 2);
    canvas.addPanel(&panels[0], 0, 0);
    canvas.addPanel(&panels[1], 1, 0);
    canvas.addPanel(&panels[2], 2, 0);
    canvas.addPanel(&panels[3], 0, 1);
    canvas.addPanel(&panels[4], 1, 1);
    canvas.addPanel(&panels[5], 2, 1);
    canvas.setup();
    canvas.setBrightness(16);

    for (int i = 0; i < colorMap.getUsed(); i++) {
        printf("Filling with color %d\n", i);
        canvas.fill(colorMap.getColor(i));
        canvas.show();
        sleep_ms(250);
    }

    canvas.fill(colorMap.getBackground());
    canvas.show();
    sleep_ms(100);

    while (1) {
        for (int i = 0; i < canvas.getWidth() - canvas.getHeight(); i++) {
            for (int j = 0; j < canvas.getHeight(); j++) {
                canvas.fill(colorMap.getBackground());
                int x = i + j;
                for (int k = 0; k < BAR_WIDTH; k++) {
                    canvas.set(x+k, j, colorMap.getColor(k));
                }                    
                canvas.show();
                sleep_ms(25);
            }
        }
    }
}
