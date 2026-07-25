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

#define STRIP_LEN 256
#define NUM_STRIPS 1
#define START_PIN 2
#define PANEL_WIDTH 8

int main() {
    stdio_init_all();

    sleep_ms(2000);

    //
    // Simple test for a single strip of pixels.
    Strip strip(START_PIN, STRIP_LEN);
    Renderer renderer(16);
    renderer.add(&strip);
    renderer.setup();

    ArrayColorMap colorMap({RGB::Red, RGB::Orange, RGB::Yellow, RGB::Green,
                            RGB::Blue, RGB::Indigo, RGB::Violet, RGB::White,
                            RGB::Gold});
    for (int i = 0; i < colorMap.getUsed(); i++) {
        printf("Filling with color %d\n", i);
        strip.fill(colorMap.getColor(i));
        renderer.render();
        sleep_ms(250);
    }

    strip.fill(colorMap.getBackground());
    renderer.render();
    sleep_ms(100);

    for (int i = 0; i < strip.getNumPixels(); i++) {
        strip.fill(colorMap.getBackground());
        strip.putPixel(RGB::Red, i);
        if (i >= 1) {
            strip.putPixel(RGB::Green, i - 1);
        }
        if (i >= 2) {
            strip.putPixel(RGB::Blue, i - 2);
        }
        renderer.render();
        sleep_ms(100);
    }

    //
    // Fill with color bands.
    printf("Filling with color bands on %d pixels\n", strip.getNumPixels());
    RGB cc = colorMap.getColor(0);
    int colorIndex = 0;
    for (int i = 0; i < strip.getNumPixels(); i++) {
        strip.putPixel(cc, i);
        if ((i + 1) % PANEL_WIDTH == 0) {
            colorIndex = (colorIndex + 1) % colorMap.getUsed();
            cc = colorMap.getColor(colorIndex);
        }
    }
    renderer.render();
    sleep_ms(1000);

    float fps = 30;
    float usPerFrame = 1e6 / fps;
    printf("usPerFrame: %f\n", usPerFrame);
    StopWatch frameWatch;
    uint32_t missedFrames = 0;
    int startPos = 0;
    int width = PANEL_WIDTH;
    int currColorIndex = 0;
    while (1) {
        frameWatch.start();
        strip.rotate(RIGHT);
        renderer.render();
        frameWatch.finish();
        uint64_t lus = frameWatch.getLastTime();
        if (lus < usPerFrame) {
            sleep_us(usPerFrame - lus);
        } else {
            missedFrames++;
        }

        if (frameWatch.count % 200 == 0) {
            printf(
                "%d frames, %.2f us/frame, %.2f us frame time  %.1f fps lus: "
                "%llu",

                frameWatch.count, usPerFrame, frameWatch.getAverageTime(), fps,
                (unsigned long long)lus);

            printf("%d blocked ", renderer.getBlockedCount());
            printf("%.2f us per DMA\n",
                   (double)renderer.getDMATime() / frameWatch.count);
        }
    }
}
