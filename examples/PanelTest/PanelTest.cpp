#include <Renderer.h>
#include <stdlib.h>

#include "Animator.h"
#include "ArrayColorMap.h"
#include "FontTwoP.h"
#include "Icicles.h"
#include "Marquees.h"
#include "Panel.h"
#include "PanelCanvas.h"
#include "ScrollTexts.h"
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
#define FPS 30

int main() {
    stdio_init_all();

    ArrayColorMap colorMap({RGB::Red, RGB::Orange, RGB::Yellow, RGB::Green,
                            RGB::Blue, RGB::Indigo, RGB::Violet, RGB::White,
                            RGB::Gold});

    ArrayColorMap xmasColors({
        RGB::Red,
        RGB::Green,
        RGB::Blue,
        RGB::White,
        RGB(213, 181, 52),  // Gold
        RGB::Purple,
        RGB::Orange,
        RGB::Silver,
        RGB(255, 20, 20),    // Bright Red
        RGB(28, 191, 38),    // Vibrant Green
        RGB(0, 33, 111),     // Navy
        RGB(255, 252, 245),  // Warm White
        RGB::Yellow,
        RGB(242, 18, 18),  // Deep Red
        RGB::FairyLightNCC,
    });

    uint8_t rColors[] = {0};
    uint8_t gColors[] = {1};
    uint8_t rgColors[] = {0, 1};
    uint8_t rwColors[] = {0, 3};
    uint8_t rgwColors[] = {0, 1, 3};
    uint8_t rgbwColors[] = {0, 1, 2, 3};
    uint8_t rgbwgColors[] = {0, 1, 2, 3, 4};

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
    canvas.setBrightness(8);

    for (int i = 0; i < colorMap.getUsed(); i++) {
        printf("Filling with color %d\n", i);
        canvas.fill(colorMap.getColor(i));
        canvas.show();
        sleep_ms(250);
    }

    canvas.fill(colorMap.getBackground());
    canvas.show();
    sleep_ms(100);

    Animator animator(&canvas, FPS);

    Font font(FontTwoPData);
    const char* text = "Hello, World!";
    uint sx = font.getWidth(text);
    ScrollText t1(&canvas, &font, text, -sx, 4, xmasColors[0],
                  RenderAngle::RENDER_0);
    t1.setClear(true);
    t1.setFrameDelay(false);
    animator.addTimed(&t1, 10000);

    Marquees fancyMarq(&canvas, &xmasColors, 5, rgbwgColors, 20, RIGHT,
                       canvas.getHeight());
    fancyMarq.setName("FMarq");
    fancyMarq.setFPS(40);

    animator.addTimed(&fancyMarq, 2000);

    // ArrayColorMap icicleMap(8);
    // Icicles icicles(&canvas, &icicleMap, 6, 6, RGB(0, 255, 0));
    // icicles.setName("Icicles");
    // animator.addTimed(&icicles, 10000);

    animator.init();

    while (true) {
        animator.step();
        if (animator.getFrameCount() % 200 == 0) {
            animator.printStats();
        }
    }
}
