#include <Renderer.h>
#include <stdlib.h>

#include "9x15B_font.h"
#include "6x10_font.h"
#include "Animator.h"
#include "ArrayColorMap.h"
#include "BDFFont.h"
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

    Panel panels[16] = {Panel(START_PIN, PANEL_WIDTH, PANEL_HEIGHT),
                       Panel(START_PIN + 1, PANEL_WIDTH, PANEL_HEIGHT),
                       Panel(START_PIN + 2, PANEL_WIDTH, PANEL_HEIGHT),
                       Panel(START_PIN + 3, PANEL_WIDTH, PANEL_HEIGHT),
                       Panel(START_PIN + 4, PANEL_WIDTH, PANEL_HEIGHT),
                       Panel(START_PIN + 5, PANEL_WIDTH, PANEL_HEIGHT),
                       Panel(START_PIN + 6, PANEL_WIDTH, PANEL_HEIGHT),
                       Panel(START_PIN + 7, PANEL_WIDTH, PANEL_HEIGHT),
                       Panel(START_PIN + 8, PANEL_WIDTH, PANEL_HEIGHT),
                       Panel(START_PIN + 9, PANEL_WIDTH, PANEL_HEIGHT),
                       Panel(START_PIN + 10, PANEL_WIDTH, PANEL_HEIGHT),
                       Panel(START_PIN + 11, PANEL_WIDTH, PANEL_HEIGHT),
                       Panel(START_PIN + 12, PANEL_WIDTH, PANEL_HEIGHT),
                       Panel(START_PIN + 13, PANEL_WIDTH, PANEL_HEIGHT),
                       Panel(START_PIN + 14, PANEL_WIDTH, PANEL_HEIGHT),
                       Panel(START_PIN + 15, PANEL_WIDTH, PANEL_HEIGHT)};

    PanelCanvas canvas(PANEL_WIDTH, PANEL_HEIGHT, 4, 4);
    for(int x = 0; x < 4; x++) {
        for(int y = 0; y < 4; y++) {
            canvas.addPanel(&panels[x + y * 4], x, y);
        }
    }
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

    for(int i = 0; i < canvas.getWidth(); i++) {
        canvas.clear();
        canvas.fillColumn(i, RGB::Red);
        canvas.show();
        sleep_ms(50);
    }

    for(int i = 0; i < canvas.getHeight(); i++) {
        canvas.clear();
        canvas.fillRow(i, RGB::Green);
        canvas.show();
        sleep_ms(50);
    }

    Animator animator(&canvas, FPS);

    BDFFont bdf(font_9x15b);
    const char* helloText = "Hello, World!";
    int sx = bdf.getWidth(helloText);

    BDFFont bdf2(font_6x10);
    const char* testText = "TEST hqgijPJ Testing the font rendering!";
    int stt = bdf2.getWidth(testText);

    ScrollText t1(&canvas, &bdf, helloText, canvas.getWidth(), 4, xmasColors[0],
                  RenderAngle::RENDER_0);
    t1.setDirection(Direction::LEFT);
    t1.setClear(true);
    t1.setFrameDelay(false);
    animator.addTimed(&t1, 10000);

    ScrollText t2(&canvas, &bdf2, testText, canvas.getWidth(), 4, xmasColors[0],
                  RenderAngle::RENDER_0);
    t2.setDirection(Direction::LEFT);
    t2.setClear(true);
    t2.setFrameDelay(false);
    animator.addTimed(&t2, 20000);

    Marquees fancyMarq(&canvas, &xmasColors, 5, rgbwgColors, 20, RIGHT,
                       canvas.getHeight());
    fancyMarq.setName("FMarq");
    fancyMarq.setFPS(40);

    animator.addTimed(&fancyMarq, 20000);

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
