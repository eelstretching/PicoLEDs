#ifndef NTP_CLIENT_H
#define NTP_CLIENT_H

#pragma once

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

typedef struct NTP_STATE {
    ip_addr_t ntp_server_address;
    bool dns_request_sent;
    struct udp_pcb *ntp_pcb;
    absolute_time_t ntp_test_time;
    alarm_id_t ntp_resend_alarm;
    time_t result;
    bool ntp_request_complete;
} NTP_t;

#define NTP_SERVER "pool.ntp.org"
#define NTP_MSG_LEN 48
#define NTP_PORT 123
#define NTP_DELTA 2208988800 // seconds between 1 Jan 1900 and 1 Jan 1970
#define NTP_TEST_TIME (30 * 1000)
#define NTP_RESEND_TIME (30 * 1000)

/// Seconds offset between UTC and EST, so we can set the RTC in eastern time.
#define NTP_EST_OFFSET (-14400)

void ntp_result(NTP_t* state, int status, time_t *result);

int64_t ntp_failed_handler(alarm_id_t id, void *user_data);

void ntp_request(NTP_t *state);

void ntp_dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg);

void ntp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);

NTP_t* ntp_init(void);

int ntp_get_time(NTP_t *state);

#endif