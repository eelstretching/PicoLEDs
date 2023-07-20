#include <stdlib.h>

#include "Canvas.h"
#include "DataAnimation.h"
#include "Fireworks.h"
#include "FireworkWipe.h"
#include "FontTwoP.h"
#include "RandomAnimation.h"
#include "RandomText.h"
#include "PacChase.h"
#include "PacWipe.h"
#include "ScrollWipe.h"
#include "Strip.h"
#include "ScoutLaw.h"
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

    //
    // Setup for networking.
    if (cyw43_arch_init_with_country(CYW43_COUNTRY_USA)) {
        printf("WiFi not init\n");
        twoP.render("WiFi init failed", 4, 4, RGB::Red);
        canvas.show();
        return 1;
    }

    cyw43_arch_enable_sta_mode();

    printf("CONNECT TO WIFI %s\n", WIFI_SSID);
    twoP.render("CONNECT TO WIFI...", 4, 4, RGB::Green);
    canvas.show();

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD,
                                           CYW43_AUTH_WPA2_AES_PSK, 60000)) {
        canvas.clear();
        twoP.render("FAIL CONNECT!!", 4, 4, RGB::Red);
        canvas.show();
        return 1;
    }

    canvas.clear();
    twoP.render("WIFI CONNECT!", 4, 4, RGB::Green);
    canvas.show();
    sleep_ms(2000);

    canvas.clear();
    twoP.render("FETCH DATA...", 4,4, RGB::Green);
    canvas.show();
    data_t *signData = fetch_data();

    rtc_init();
    rtc_set_datetime(&signData->dt);

    cyw43_arch_deinit();

    StopWatch aw;
    StopWatch sw;

    //
    // A couple of animations.
    TextAnimation text(&canvas, &twoP, 5000);
    TextElement t103("TROOPS 103 AND 511", 10, 9, RGB::ForestGreen);
    TextElement burl("BURLINGTON, MA", 10, 0, RGB::Gold);
    text.add(&t103);
    text.add(&burl);

    RandomText randText(&canvas, &twoP, 6000);

    TextElement prep("BE PREPARED", 10, 8, RGB::Gold);
    TextElement turn("DO A GOOD TURN\nDAILY", 10, 8, RGB::Gold);
    TextElement fire("BE CAREFUL WITH\nFIRE", 10, 8, RGB::Gold);
    TextElement essr("EAT. SLEEP.\nSCOUT. REPEAT.", 10, 8, RGB::Gold);
    TextElement better("LEAVE IT BETTER\nTHAN YOU FOUND IT", 10, 8, RGB::Gold);
    TextElement doubt("WHEN IN DOUBT\nHELP OUT", 10, 8, RGB::Gold);
    TextElement advent("BE ADVENTUROUS", 10, 8, RGB::Gold);
    TextElement good("GOOD DEEDS\nBRIGHTEN THE WORLD", 10, 8, RGB::Gold);
    randText.add(&prep);
    randText.add(&turn);
    randText.add(&fire);
    randText.add(&essr);
    randText.add(&better);
    randText.add(&doubt);
    randText.add(&advent);
    randText.add(&good);

    ScoutLaw law(&canvas, &twoP, 500);

    DataAnimation wxData(&canvas, &twoP, 5000, signData);
    TimeAnimation time(&canvas, &twoP, 8000);

    ScrollWipe upWipe(&canvas, ScrollDirection::SCROLL_UP);
    upWipe.setExtraFrames(20);

    ScrollWipe downWipe(&canvas, ScrollDirection::SCROLL_DOWN);
    downWipe.setExtraFrames(20);

    ScrollWipe leftWipe(&canvas, ScrollDirection::SCROLL_LEFT);
    leftWipe.setExtraFrames(20);

    ScrollWipe rightWipe(&canvas, ScrollDirection::SCROLL_RIGHT);
    rightWipe.setExtraFrames(20);

    FireworkWipe fww(&canvas);
    Fireworks fireworks(&canvas, fww.getFireworks(), fww.getNumFireWorks());
    TimedAnimation timedFireworks(&fireworks, 20000);

    PacWipe pacWipe(&canvas);
    PacChase pacChase(&pacWipe);


    RandomAnimation wipes(&canvas);
    wipes.add(&upWipe);
    wipes.add(&downWipe);
    wipes.add(&leftWipe);
    wipes.add(&rightWipe);
    wipes.add(&fww);
    wipes.add(&pacWipe);

    Animator animator(&canvas, 30);
    animator.add(&text);
    animator.add(&wipes);
    animator.add(&wxData);
    animator.add(&wipes);
    animator.add(&time);
    animator.add(&wipes);
    animator.add(&randText);
    animator.add(&wipes);
    animator.add(&law);
    animator.add(&wipes);
    animator.add(&pacChase);
    animator.add(&timedFireworks);
    animator.add(&pacWipe);

    animator.init();

    // void *foo = malloc(16*1024);
    // if(foo != NULL) {
    //     printf("Malloced 16k!\n");
    //     free(foo);
    // }

    int n = 0;
    datetime_t dt;
    while (1) {
        animator.step();
        n++;
        if (n % 1000 == 0) {
            rtc_get_datetime(&dt);
            printf(
                "Time: %04d/%02d/%02d %02d:%02d:%02d %d frames run, %.2f "
                "us/frame, %d missed frames\n",
                dt.year, dt.month, dt.day, dt.hour, dt.min, dt.sec, n,
                animator.getAverageFrameTimeUS(), animator.getMissedFrames());
        }
    }
}