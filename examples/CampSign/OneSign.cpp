#include <stdlib.h>

#include "Canvas.h"
#include "DataAnimation.h"
#include "FireworkWipe.h"
#include "FontTwoP.h"
#include "PacWipe.h"
#include "PacChase.h"
#include "RandomAnimation.h"
#include "RandomText.h"
#include "ScoutLaw.h"
#include "ScrollWipe.h"
#include "Strip.h"
#include "TextAnimation.h"
#include "TimeAnimation.h"
#include "TimedAnimation.h"
#include "View.h"
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
    Canvas canvas(138, 16);
    View view(138);
    Strip strips[] = {Strip(2, 552), Strip(3, 552), Strip(4, 552),
                      Strip(5, 552)};
    view.add(strips[0]);
    view.add(strips[1]);
    view.add(strips[2]);
    view.add(strips[3]);
    canvas.setView(&view, 0, 0);

    int delay = 100;

    int msPerFrame = 1000 / 60;

    //
    // Init to clear the strips and show they're working while rendering's
    // busted.
    for (int i = 0; i < 4; i++) {
        strips[i].fill(RGB::Green);
        strips[i].show();
    }
    for (int i = 0; i < 4; i++) {
        sleep_ms(delay);
        strips[i].fill(RGB::Black, 0, 138);
        strips[i].show();

        sleep_ms(delay);
        strips[i].fill(RGB::Black, 138, 276);
        strips[i].show();

        sleep_ms(delay);
        strips[i].fill(RGB::Black, 276, 414);
        strips[i].show();

        sleep_ms(delay);
        strips[i].fill(RGB::Black, 414, 552);
        strips[i].show();
    }

    sleep_ms(delay);
    canvas.clear();
    Font twoP(&canvas, FontTwoPData);

    PacChase chase(&canvas);
    Animator animator(&canvas, 30);
    animator.add(&chase);
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