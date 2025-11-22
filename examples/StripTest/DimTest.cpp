#include <Renderer.h>
#include <stdlib.h>

#include "FadeColorMap.h"
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

    ArrayColorMap baseMap(4);
    baseMap.addColor(RGB(0, 64, 0));
    baseMap.addColor(RGB(128, 0, 0));
    ColorMap* fadeMap = new FadeColorMap(&baseMap, 2, 32, 28);
    RGB curr = RGB::Red;
    strip.fill(fadeMap->getBackgroundIndex());
    renderer.render(fadeMap);

    float fps = 10;
    float usPerFrame = 1e6 / fps;
    StopWatch frameWatch;
    uint32_t missedFrames = 0;

    while (true) {
        strip.fill(fadeMap->getBackgroundIndex());
        for (int i = 32; i < fadeMap->getUsed(); i++) {
            frameWatch.start();
            strip.fill(i, 0, 100);
            strip.putPixel(0, 100 + (i-32));
            renderer.render(fadeMap);
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
