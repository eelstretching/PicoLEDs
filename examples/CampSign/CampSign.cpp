#include <stdlib.h>

#include "Canvas.h"
#include "DataAnimation.h"
#include "FireworkWipe.h"
#include "Fireworks.h"
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

#define NUM_STRIPS 16
#define START_PIN 2
#define STRIP_LEN 137

int main() {
    //
    // Set up to printf stuff.
    stdio_init_all();

    Strip *strips[NUM_STRIPS];
    int ns = NUM_STRIPS;
    int pin = START_PIN;
    int pins[NUM_STRIPS];
    for (int i = 0; i < ns; i++) {
        pins[i] = pin;
        strips[i] = new Strip(pin++, STRIP_LEN);
        strips[i]->setFractionalBrightness(32);
    }

    Canvas canvas(STRIP_LEN);
    for (int i = 0; i < ns; i++) {
        canvas.add(*strips[i]);
    }
    canvas.setup();
    canvas.clear();
    canvas.show();

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

    data_t *signData = nullptr;

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD,
                                           CYW43_AUTH_WPA2_AES_PSK, 60000)) {
        canvas.clear();
        twoP.render("FAIL CONNECT!!", 4, 4, RGB::Red);
        canvas.show();
    } else {
        canvas.clear();
        twoP.render("WIFI CONNECT!", 4, 4, RGB::Green);
        canvas.show();
        sleep_ms(2000);

        canvas.clear();
        twoP.render("FETCH DATA...", 4, 4, RGB::Green);
        canvas.show();
        signData = fetch_data();

        rtc_init();
        rtc_set_datetime(&signData->dt);
    }

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
    TextElement better("LEAVE IT BETTER\nTHAN YOU FOUND IT", 2, 8, RGB::Gold);
    TextElement doubt("WHEN IN DOUBT,\nHELP OUT", 10, 8, RGB::Gold);
    TextElement advent("BE ADVENTUROUS", 10, 8, RGB::Gold);
    TextElement good("GOOD DEEDS\nBRIGHTEN THE WORLD", 2, 8, RGB::Gold);
    randText.add(&prep);
    randText.add(&turn);
    randText.add(&fire);
    randText.add(&essr);
    randText.add(&better);
    randText.add(&doubt);
    randText.add(&advent);
    randText.add(&good);

    ScoutLaw law(&canvas, &twoP, 500);

    DataAnimation *wxData = nullptr;
    TimeAnimation *time = nullptr;
    if (signData != nullptr) {
        wxData = new DataAnimation(&canvas, &twoP, 5000, signData);
        time = new TimeAnimation(&canvas, &twoP, 8000);
    }

    ScrollWipe upWipe(&canvas, Direction::UP);
    upWipe.setExtraFrames(20);

    ScrollWipe downWipe(&canvas, Direction::DOWN);
    downWipe.setExtraFrames(20);

    ScrollWipe leftWipe(&canvas, Direction::LEFT);
    leftWipe.setExtraFrames(20);

    ScrollWipe rightWipe(&canvas, Direction::RIGHT);
    rightWipe.setExtraFrames(20);

    FireworkWipe fww(&canvas);
    Fireworks fireworks(&canvas, fww.getFireworks(), fww.getNumFireWorks());
    TimedAnimation timedFireworks(&fireworks, 20000);

    PacWipe pacWipe(&canvas);
    PacChase pacChase(&pacWipe);

    RainbowWipe rainbow(&canvas);

    RandomAnimation wipes(&canvas);
    wipes.add(&upWipe);
    wipes.add(&downWipe);
    wipes.add(&leftWipe);
    wipes.add(&rightWipe);
    wipes.add(&fww);
    wipes.add(&pacWipe);
    wipes.add(&rainbow);

    Animator animator(&canvas, 30);
    animator.add(&text);
    animator.add(&wipes);
    if (wxData != nullptr) {
        animator.add(wxData);
    }
    animator.add(&wipes);
    if (time != nullptr) {
        animator.add(time);
    }
    animator.add(&wipes);
    animator.add(&randText);
    animator.add(&wipes);
    animator.add(&law);
    animator.add(&downWipe);
    animator.add(&pacChase);
    animator.add(&timedFireworks);
    animator.add(&rainbow);

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
        if (n % 500 == 0) {
            rtc_get_datetime(&dt);
            printf(
                "Time: %04d/%02d/%02d %02d:%02d:%02d %d frames run, %.2f "
                "us/frame, %.2f us/show %d missed frames\n",
                dt.year, dt.month, dt.day, dt.hour, dt.min, dt.sec, n,
                animator.getAverageFrameTimeUS(),
                animator.getAverageShowTimeUS(), animator.getMissedFrames());
        }
    }
}