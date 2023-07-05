/*
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tls_http.h"

#include <string.h>
#include <time.h>

#include "lwip/altcp_tcp.h"
#include "lwip/altcp_tls.h"
#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

err_t tls_client_close(void *arg) {
    tls_client_t *state = (tls_client_t *)arg;
    err_t err = ERR_OK;

    state->complete = true;
    if (state->pcb != NULL) {
        altcp_arg(state->pcb, NULL);
        altcp_poll(state->pcb, NULL, 0);
        altcp_recv(state->pcb, NULL);
        altcp_err(state->pcb, NULL);
        err = altcp_close(state->pcb);
        if (err != ERR_OK) {
            printf("close failed %d, calling abort\n", err);
            altcp_abort(state->pcb);
            err = ERR_ABRT;
        }
        state->pcb = NULL;
    }
    return err;
}

err_t tls_client_connected(void *arg, struct altcp_pcb *pcb, err_t err) {
    tls_client_t *state = (tls_client_t *)arg;
    if (err != ERR_OK) {
        printf("connect failed %d\n", err);
        return tls_client_close(state);
    }

    printf("connected to server, sending request\n");
    err = altcp_write(state->pcb, state->http_request,
                      strlen(state->http_request), TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        printf("error writing data, err=%d", err);
        return tls_client_close(state);
    }

    return ERR_OK;
}

err_t tls_client_poll(void *arg, struct altcp_pcb *pcb) {
    tls_client_t *state = (tls_client_t *)arg;
    printf("timed out\n");
    state->error = PICO_ERROR_TIMEOUT;
    return tls_client_close(arg);
}

void tls_client_err(void *arg, err_t err) {
    tls_client_t *state = (tls_client_t *)arg;
    printf("tls_client_err %d\n", err);
    tls_client_close(state);
    state->error = PICO_ERROR_GENERIC;
}

err_t tls_client_recv(void *arg, struct altcp_pcb *pcb, struct pbuf *p,
                      err_t err) {
    tls_client_t *state = (tls_client_t *)arg;
    if (!p) {
        printf("connection closed\n");
        return tls_client_close(state);
    }

    if (p->tot_len > 0) {
        if (state->buff == NULL) {
            state->buff = calloc(p->tot_len + 1, sizeof(char));
            state->buffSize = p->tot_len + 1;
            state->buffPos = 0;
        } else if (state->buffPos + p->tot_len + 1 >= state->buffSize) {
            //
            // Need to realloc the buffer in the state for a longer response.
            char *save = state->buff;
            state->buff = calloc(state->buffPos + p->tot_len + 1, sizeof(char));
            state->buffSize = state->buffPos + p->tot_len + 1;
            memcpy(state->buff, save, state->buffPos * sizeof(char));
            free(save);
        }
        char buf[p->tot_len + 1];

        //
        // Copy the data from the pbuf to our buffer, and make sure it's
        // null-terminated.
        uint16_t n = pbuf_copy_partial(p, &state->buff[state->buffPos], p->tot_len, 0);
        printf("Copied %d/%d characters to state\n", n, p->tot_len);
        state->buffPos += n;
        state->buff[state->buffPos] = 0;

        altcp_recved(pcb, p->tot_len);
    }
    pbuf_free(p);

    return ERR_OK;
}

void tls_client_connect_to_server_ip(const ip_addr_t *ipaddr,
                                     tls_client_t *state) {
    err_t err;
    u16_t port = 443;

    printf("connecting to server IP %s port %d\n", ipaddr_ntoa(ipaddr), port);
    err = altcp_connect(state->pcb, ipaddr, port, tls_client_connected);
    if (err != ERR_OK) {
        fprintf(stderr, "error initiating connect, err=%d\n", err);
        tls_client_close(state);
    }
}

void tls_client_dns_found(const char *hostname, const ip_addr_t *ipaddr,
                          void *arg) {
    if (ipaddr) {
        printf("DNS resolving complete\n");
        tls_client_connect_to_server_ip(ipaddr, (tls_client_t *)arg);
    } else {
        printf("error resolving hostname %s\n", hostname);
        tls_client_close(arg);
    }
}

bool tls_client_open(const char *hostname, void *arg) {
    err_t err;
    ip_addr_t server_ip;
    tls_client_t *state = (tls_client_t *)arg;

    state->pcb = altcp_tls_new(state->tls_config, IPADDR_TYPE_ANY);
    if (!state->pcb) {
        printf("failed to create pcb\n");
        return false;
    }

    altcp_arg(state->pcb, state);
    altcp_poll(state->pcb, tls_client_poll, state->timeout * 2);
    altcp_recv(state->pcb, tls_client_recv);
    altcp_err(state->pcb, tls_client_err);

    /* Set SNI */
    mbedtls_ssl_set_hostname(altcp_tls_context(state->pcb), hostname);

    printf("resolving %s\n", hostname);

    // cyw43_arch_lwip_begin/end should be used around calls into lwIP to ensure
    // correct locking. You can omit them if you are in a callback from lwIP.
    // Note that when using pico_cyw_arch_poll these calls are a no-op and can
    // be omitted, but it is a good practice to use them in case you switch the
    // cyw43_arch type later.
    cyw43_arch_lwip_begin();

    err = dns_gethostbyname(hostname, &server_ip, tls_client_dns_found, state);
    if (err == ERR_OK) {
        /* host is in DNS cache */
        tls_client_connect_to_server_ip(&server_ip, state);
    } else if (err != ERR_INPROGRESS) {
        printf("error initiating DNS resolving, err=%d\n", err);
        tls_client_close(state->pcb);
    }

    cyw43_arch_lwip_end();

    return err == ERR_OK || err == ERR_INPROGRESS;
}

// Perform initialisation
tls_client_t *tls_client_init(void) {
    tls_client_t *state = calloc(1, sizeof(tls_client_t));
    if (!state) {
        printf("failed to allocate state\n");
        return NULL;
    }

    return state;
}

bool fetch_https(tls_client_t *state, uint8_t *cert, size_t cert_len,
                 const char *server, const char *request, int timeout) {
    /* No CA certificate checking */
    state->tls_config = altcp_tls_create_config_client(cert, cert_len);
    assert(state->tls_config);

    // mbedtls_ssl_conf_authmode(&tls_config->conf,
    // MBEDTLS_SSL_VERIFY_OPTIONAL);

    state->http_request = request;
    state->timeout = timeout;
    if (!tls_client_open(server, state)) {
        return false;
    }
    while (!state->complete) {
        // if you are not using pico_cyw43_arch_poll, then WiFI driver and lwIP
        // work is done via interrupt in the background. This sleep is just an
        // example of some (blocking) work you might be doing.
        sleep_ms(1000);
    }
    int err = state->error;
    altcp_tls_free_config(state->tls_config);
    return err == 0;
}
