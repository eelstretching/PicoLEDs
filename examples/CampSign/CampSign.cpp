#include "Canvas.h"
#include "Firework.h"
#include "Strip.h"
#include "View.h"
#include "sun.h"

extern "C" {
#include "ntp_client.h"
}

extern "C" {
#include "tls_http.h"
}

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

void set_rtc() {
    //
    // Initialize NTP and set the RTC.
    NTP_t *ntpState = ntp_init();
    StopWatch sw;
    sw.start();
    ntp_get_time(ntpState);
    while (!ntpState->ntp_request_complete) {
        sleep_ms(100);
    }
    sw.finish();
    printf("NTP sync took %.2f ms\n", sw.getLastTimeMS());

    struct tm utc;
    time_t est = ntpState->result + NTP_EST_OFFSET;
    gmtime_r(&est, &utc);
    datetime_t dt = {.year = (int16_t)(utc.tm_year + 1900),
                     .month = (int8_t)(utc.tm_mon + 1),
                     .day = (int8_t)utc.tm_mday,
                     .dotw = (int8_t)utc.tm_wday,
                     .hour = (int8_t)utc.tm_hour,
                     .min = (int8_t)utc.tm_min,
                     .sec = (int8_t)utc.tm_sec};
    rtc_init();
    rtc_set_datetime(&dt);
    free(ntpState);
}

int main() {
    //
    // Set up to printf stuff.
    stdio_init_all();

    //
    // Setup for networking.
    if (cyw43_arch_init_with_country(CYW43_COUNTRY_USA)) {
        printf("WiFi failed to initialise\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD,
                                           CYW43_AUTH_WPA2_AES_PSK, 60000)) {
        printf("WiFi failed to connect\n");
        return 1;
    }

    set_rtc();

    //
    // Get sunrise, sunset.
    tls_client_t *state = tls_client_init();
    if (!state) {
        return false;
    }

    bool pass = fetch_https(state, NULL, 0, SUN_CLIENT_SERVER,
                            SUN_CLIENT_HTTP_REQUEST, SUN_CLIENT_TIMEOUT_SECS);
    if (pass) {
        printf("Fetched data OK\n");
        printf("Data:\n");
        printf(state->buff);
        printf("\n");
    } else {
        printf("Well, shit\n");
    }

    free(state);

    datetime_t dt;
    while (1) {
        rtc_get_datetime(&dt);
        printf("RTC running: %s RTC time is: %04d/%02d/%02d %02d:%02d:%02d\n",
               rtc_running() ? "yes" : "no", dt.year, dt.month, dt.day, dt.hour,
               dt.min, dt.sec);
        sleep_ms(2000);
    }

    cyw43_arch_deinit();
}