#include <Renderer.h>
#include <stdlib.h>

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
#define NUM_STRIPS 1
#define START_PIN 2
#define WIDTH 10

int main() {
    stdio_init_all();

    sleep_ms(2000);

    //
    // Simple test for a single strip of pixels.
    Strip strip(START_PIN, STRIP_LEN);
    strip.setColorOrder(ColorOrder::ORGB);
    Renderer renderer;
    renderer.add(strip);
    renderer.setup();

    ColorMap dimMap(128);
    dimMap.addColor(RGB(0, 64, 0));
    RGB curr = RGB::Red;
    for (int i = 0; i < 32; i++) {
        dimMap.addColor(curr);
        curr.fadeToBlackBy(40);
    }
    strip.fill(dimMap.getBackgroundIndex());
    renderer.render(&dimMap);

    char buffer[32];
    for (int i = 0; i < dimMap.getUsed(); i++) {
        printf("Color %d: %s\n", i, dimMap[i].toString(buffer, 32));
    }

    float fps = 10;
    float usPerFrame = 1e6 / fps;
    StopWatch frameWatch;
    uint32_t missedFrames = 0;

    while (true) {
        strip.fill(dimMap.getBackgroundIndex());
        for (int i = 1; i < dimMap.getUsed(); i++) {
            frameWatch.start();
            strip.fill(i, 0, 100);
            strip.putPixel(0, 100 + i);
            renderer.render(&dimMap);
            frameWatch.finish();
            uint64_t lus = frameWatch.getLastTime();
            if (lus < usPerFrame) {
                sleep_us(usPerFrame - lus);
            } else {
                missedFrames++;
            }
            if (frameWatch.count % 200 == 0) {
                printf(
                    "%d frames, %.2f us/frame, %.2f us frame time  %.1f fps ",

                    frameWatch.count, usPerFrame, frameWatch.getAverageTime());

                printf("%d blocked ", renderer.getBlockedCount());
                printf("%.2f us per DMA\n",
                       (double)renderer.getDMATime() / frameWatch.count);
            }
        }
    }
}
