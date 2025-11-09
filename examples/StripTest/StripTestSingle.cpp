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

#define STRIP_LEN 200
#define NUM_STRIPS 4
#define START_PIN 2
#define WIDTH 10

int main() {
    stdio_init_all();

    //
    // Simple test for a single strip of pixels.
    Strip* strips[NUM_STRIPS];
    Renderer renderer;
    int ns = NUM_STRIPS;
    int pin = START_PIN;
    for (int i = 0; i < ns; i++) {
        strips[i] = new Strip(pin++, STRIP_LEN);
        strips[i]->setColorOrder(ColorOrder::ORGB);
        renderer.add(*strips[i]);
    }
    printf("Calling setup\n");
    renderer.setup();
    uint32_t delay = 600;

    sleep_ms(2000);

    // ColorMap colorMap({0xff0000, 0x00ff00, 0x0000ff, 0xffffff});
    ColorMap colorMap({RGB::Red, RGB::Green, RGB::Blue, RGB::White, RGB::Yellow,
                       RGB::Purple, RGB::GhostWhite, RGB::DarkViolet,
                       RGB::FireBrick});
                       colorMap.setBrightness(16);

    int dirs[NUM_STRIPS];
    for (int i = 0; i < NUM_STRIPS; i++) {
        //
        // When I originally wrote the code, the even directions were 1, so
        // there you go.
        dirs[i] = (i + 1) % 2;
    }

    int width = WIDTH;

    int posns[NUM_STRIPS];
    for (int i = 0; i < NUM_STRIPS; i++) {
        posns[i] = i % 2 == 0 ? 0 : ((int)strips[0]->getNumPixels() - width);
    }

    for(int i = 0; i < ns; i++) {
        strips[i]->fill(3);
    }
    renderer.render(&colorMap);
    sleep_ms(2000);

    for (int i = 0; i < 2; i++) {
        for (int k = 0; k < colorMap.getUsed(); k++) {
            printf("Filling with %d\n", k);
            for (int j = 0; j < ns; j++) {
                strips[j]->fill(k);
            }
            renderer.render(&colorMap);
            sleep_ms(delay);
        }

        for (int j = 0; j < ns; j++) {
            strips[j]->fill(colorMap.getBackgroundIndex());
        }
        renderer.render(&colorMap);
        sleep_ms(delay);
    }

    float fps = 60;
    float usPerFrame = 1e6 / fps;
    StopWatch frameWatch;
    uint32_t missedFrames = 0;

    while (1) {
        frameWatch.start();
        for (int s = 0; s < ns; s++) {
            if (dirs[s] == 1) {
                if (posns[s] > 0) {
                    strips[s]->putPixel(colorMap.getBackgroundIndex(),
                                        posns[s] - 1);
                }
                for (int i = posns[s]; i < posns[s] + width; i++) {
                    strips[s]->putPixel(s, i);
                }
                posns[s]++;
                if (posns[s] == strips[s]->getNumPixels() - width) {
                    dirs[s] = 0;
                }
            } else {
                if (posns[s] < strips[s]->getNumPixels() - width) {
                    strips[s]->putPixel(colorMap.getBackgroundIndex(),
                                        posns[s] + width);
                }
                for (int i = posns[s]; i < posns[s] + width; i++) {
                    strips[s]->putPixel(s, i);
                }
                posns[s]--;
                if (posns[s] == -1) {
                    dirs[s] = 1;
                    posns[s] = 0;
                }
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
