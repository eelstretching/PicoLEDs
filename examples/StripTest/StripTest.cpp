#include <Renderer.h>
#include <stdlib.h>

#include "ArrayColorMap.h"
#include "StopWatch.h"
#include "Strip.h"
#include "colorutils.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/printf.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/types.h"

#define STRIP_LEN 300
#define NUM_STRIPS 7
#define START_PIN 2
#define WIDTH 10

int main() {
    stdio_init_all();

    //
    // Simple test for a few strips of pixels.
    Strip* strips[NUM_STRIPS];
    Renderer renderer;
    int ns = NUM_STRIPS;
    int pin = START_PIN;
    for (int i = 0; i < ns; i++) {
        strips[i] = new Strip(pin++, STRIP_LEN);
        renderer.add(*strips[i]);
    }
    renderer.setup();

    ArrayColorMap colorMap({RGB::Red, RGB::Orange, RGB::Yellow, RGB::Green,
                       RGB::Blue, RGB::Indigo, RGB::Violet, RGB::White});
    colorMap.setBrightness(64);

    for (int c = 0; c < colorMap.getUsed(); c++) {
        for (int i = 0; i < ns; i++) {
            strips[i]->fill(c);
        }
        renderer.render(&colorMap);
        sleep_ms(250);
    }
    for (int i = 0; i < ns; i++) {
        strips[i]->fill(colorMap.getBackgroundIndex());
    }
    renderer.render(&colorMap);
    sleep_ms(100);

    //
    // Fill with color bands.
    for (int s = 0; s < ns; s++) {
        Strip& strip = *strips[s];
        uint8_t cc = 0;
        for (int i = 0; i < strip.getNumPixels(); i++) {
            strip.putPixel(cc, i);
            if ((i + 1) % WIDTH == 0) {
                printf("Color change at pixel %d\n", i);
                cc = (cc + 1) % colorMap.getUsed();
            }
        }
    }
    renderer.render(&colorMap);
    sleep_ms(1000);

    float fps = 40;
    float usPerFrame = 1e6 / fps;
    StopWatch frameWatch;
    uint32_t missedFrames = 0;
    int startPos = 0;
    int width = WIDTH;
    int currColorIndex = 0;
    while (1) {
        frameWatch.start();
        for (int s = 0; s < ns; s++) {
            if (s % 2 == 0) {
                strips[s]->rotate(LEFT);
            } else {
                strips[s]->rotate(RIGHT);
            }
        }

        renderer.render(&colorMap);
        frameWatch.finish();
        uint64_t lus = frameWatch.getLastTime();
        if (lus < usPerFrame) {
            sleep_us(usPerFrame - lus);
        } else {
            missedFrames++;
        }

        if (frameWatch.count % 200 == 0) {
            printf("%d frames, %.2f us/frame, %.2f us frame time  %.1f fps ",

                   frameWatch.count, usPerFrame, frameWatch.getAverageTime());

            printf("%d blocked ", renderer.getBlockedCount());
            printf("%.2f us per DMA\n",
                   (double)renderer.getDMATime() / frameWatch.count);
        }
    }

}
