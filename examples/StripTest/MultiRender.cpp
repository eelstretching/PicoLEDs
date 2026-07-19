#include <stdlib.h>

#include <string>

#include "Strip.h"
#include "Renderer.h"
#include "colorutils.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/printf.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/types.h"
#include "ArrayColorMap.h"
#define NUM_STRIPS 5
#define START_PIN 2
#define STRIP_LEN 400
#define CANVAS_WIDTH 100
#define BRIGHTNESS 32
#define FPS 30

Strip* strips[NUM_STRIPS];
uint8_t pattern[100];

void setGoodPattern(Strip *strip, int p) {
    if(p == 0 || p == 200) {
        for(int i = 0; i < 100; i++) {
            strip->putPixel(pattern[i], p++);
        }
    } else {
        for(int i = 99; i >= 0; i--) {
            strip->putPixel(pattern[i], p++);
        }
    }
}

void setGoodState() {
    for(int s = 0; s < NUM_STRIPS; s++) {
        setGoodPattern(strips[s], s * 100);
    }
}
int main() {
    stdio_init_all();

    Renderer renderer;

    //
    // A canvas made out of strips.
    int ns = NUM_STRIPS;
    int pin = START_PIN;
    int pins[NUM_STRIPS];
    for (int i = 0; i < ns; i++) {
        pins[i] = pin;
        //
        // First strip is a little short at the moment.
        strips[i] = new Strip(pin++, STRIP_LEN, StripType::WS2812);
        strips[i]->setColorOrder(ColorOrder::OGRB);
        renderer.add(strips[i]);
    }

    for(int i = 0; i < 12; i++) {
        pattern[i] = 4;
    }
    for(int i = 12; i < 32; i++) {
        pattern[i] = 0;
    }
    for(int i = 32; i < 52; i++) {
        pattern[i] = 1;
    }
    for(int i = 52; i < 72; i++) {
        pattern[i] = 2;
    }
    for(int i = 72; i < 92; i++) {
        pattern[i] = 3;
    }
    for(int i = 92; i < 100; i++) {
        pattern[i] = 4;
    }

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

    ArrayColorMap dimXmasColors(xmasColors);
    dimXmasColors.setBrightness(16);

    setGoodState();

    char buff[40];
    for(int i = 0; i < 5; i++) {
        printf("Color %d: %s\n", i, dimXmasColors[i].toString(buff, 40));
    }

    int count = 0;
    while (true) {
        for(int i = 0; i < ns; i++) {
            for(int p = 0; p < 4; p ++) {
                if(p % 2 == 0) {
                    if(count % 2 == 0) {
                        strips[i]->rotateRight(p * 100, (p + 1) * 100);
                    } else {
                        strips[i]->rotateLeft(p * 100, (p + 1) * 100);
                    }
                } else {
                    if(count % 2 == 0) {
                        strips[i]->rotateLeft(p * 100, (p + 1) * 100);
                    } else {
                        strips[i]->rotateRight(p * 100, (p + 1) * 100);
                    }
                }
            }
        }
        count++;
        renderer.render();
        sleep_ms(1000);
    }
}
