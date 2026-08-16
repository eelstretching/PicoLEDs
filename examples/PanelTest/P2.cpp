#include <Renderer.h>
#include <stdlib.h>

#include "ArrayColorMap.h"
#include "StopWatch.h"
#include "Panel.h"
#include "colorutils.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/printf.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/types.h"

#define STRIP_LEN 256
#define START_PIN 2
#define NUM_PANELS 16
#define BAND_WIDTH 8
#define PANEL_WIDTH 32
#define PANEL_HEIGHT 8

#define START_FILL_PANEL 0

int main() {
    stdio_init_all();

    //
    // Simple test for a few strips of pixels.
    Panel* panels[NUM_PANELS];
    Renderer renderer(8);
    int ns = NUM_PANELS;
    int pin = START_PIN;
    for (int i = 0; i < ns; i++) {
        printf("Creating strip %d on pin %d\n", i, pin);
        panels[i] = new Panel(pin++, PANEL_WIDTH, PANEL_HEIGHT);
        renderer.add(panels[i]);
    }
    renderer.setup();
    renderer.setBrightness(8);

    ArrayColorMap colorMap({RGB::Red, RGB::Orange, RGB::Yellow, RGB::Green,
                            RGB::Blue, RGB::Indigo, RGB::Violet, RGB::White,
                            RGB::Gold});

    for (int c = 0; c < colorMap.getUsed(); c++) {
        for (int i = 0; i < ns; i++) {
            panels[i]->fill(colorMap.getColor(c));
        }
        renderer.render();
        sleep_ms(250);
    }
    for (int i = 0; i < ns; i++) {
        panels[i]->fill(colorMap.getBackground());
    }
    renderer.render();
    sleep_ms(100);

    //
    // Fill with color bands.
    for (int s = START_FILL_PANEL; s < ns; s++) {
        Strip& strip = *panels[s];
        uint8_t cc = 0;
        for (int i = 0; i < strip.getNumPixels(); i++) {
            strip.putPixel(colorMap.getColor(cc), i);
            if ((i + 1) % BAND_WIDTH == 0) {
                cc = (cc + 1) % colorMap.getUsed();
            }
        }
    }
    renderer.render();
    sleep_ms(1000);

    float fps = 10;
    float usPerFrame = 1e6 / fps;
    StopWatch frameWatch;
    uint32_t missedFrames = 0;
    int startPos = 0;
    int width = BAND_WIDTH;
    int currColorIndex = 0;
    while (1) {
        frameWatch.start();
        for (int s = START_FILL_PANEL; s < ns; s++) {
            if (s % 2 == 0) {
                panels[s]->rotateLeft();
            } else {
                panels[s]->rotateRight();
            }   
        }

        renderer.render();
        frameWatch.finish();
        uint64_t lus = frameWatch.getLastTime();
        if (lus < usPerFrame) {
            sleep_us(usPerFrame - lus);
        } else {
            missedFrames++;
        }

        // printf("Frame %d\n", frameWatch.count);        

        if (frameWatch.count % 200 == 0) {
            printf("%d frames, %.2f us/frame, %.2f us frame time  %.1f fps ",

                   frameWatch.count, usPerFrame, frameWatch.getAverageTime(), 1e6 / frameWatch.getAverageTime());

            printf("%d blocked ", renderer.getBlockedCount());
            printf("%.2f us per DMA\n",
                   (double)renderer.getDMATime() / frameWatch.count);
        }
    }

}
