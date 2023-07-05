#ifndef TLS_HTTP_H
#define TLS_HTTP_H

#pragma once

#include "pico/stdlib.h"

typedef struct tls_client_t_ {
    struct altcp_tls_config *tls_config;
    struct altcp_pcb *pcb;
    bool complete;
    int error;
    const char *http_request;
    char *buff;
    uint buffSize;
    uint buffPos;
    int timeout;
} tls_client_t;

tls_client_t *tls_client_init(void);

bool fetch_https(tls_client_t *state, uint8_t *cert, size_t cert_len,
                 const char *server, const char *request,
                 int timeout);

#endif
