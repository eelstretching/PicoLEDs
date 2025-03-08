#include <stdlib.h>

#include "Canvas.h"
#include "DataAnimation.h"
#include "FireworkWipe.h"
#include "FontTwoP.h"
#include "PacChase.h"
#include "PacWipe.h"
#include "RainbowWipe.h"
#include "RandomAnimation.h"
#include "RandomText.h"
#include "ScoutLaw.h"
#include "ScrollWipe.h"
#include "Strip.h"
#include "TextAnimation.h"
#include "TimeAnimation.h"
#include "TimedAnimation.h"
#include "colorutils.h"
#include "data.h"
#include "hardware/rtc.h"
#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "pico/cyw43_arch.h"
#include "pico/printf.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/types.h"
#include "pico/util/datetime.h"

int main() {
    //
    // Set up to printf stuff.
    stdio_init_all();

    //
    // A canvas and a view made out of strips.
    Canvas canvas(138);
    Strip strips[] = {Strip(2, 552), Strip(3, 552), Strip(4, 552),
                      Strip(5, 552)};
    canvas.add(strips[0]);
    canvas.add(strips[1]);
    canvas.add(strips[2]);
    canvas.add(strips[3]);

    int delay = 100;

    HSV hsv(160, 240, 255);
    RGB rgb(hsv);

    printf("h: %d s: %d v: %d r: %d g: %d b: %d\n", hsv.h, hsv.s, hsv.v, rgb.r,
           rgb.g, rgb.b);


    sleep_ms(delay);
    canvas.clear();

    Font twoP(&canvas, FontTwoPData);

    TextAnimation text(&canvas, &twoP, 1000);
    text.add(new TextElement("THIS IS TEXT", 20, 8, RGB::BlueViolet));
    text.add(new TextElement("FOO", 0, 0, RGB::Green));
    text.add(new TextElement("BAR", 40, 0, RGB::CadetBlue));

    ScrollWipe sw = ScrollWipe(&canvas, Direction::DOWN);
    RainbowWipe rw = RainbowWipe(&canvas);
    Animator animator(&canvas, 30);
    animator.add(&text);
    animator.add(&rw);
    animator.init();

    int n = 0;
    while (1) {
        animator.step();
        n++;
        if (n % 1000 == 0) {
            printf(
                "%d frames run, %.2f "
                "us/frame, %d missed frames\n",
                n, animator.getAverageFrameTimeUS(),
                animator.getMissedFrames());
        }
    }
}