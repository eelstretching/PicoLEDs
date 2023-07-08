#include "data.h"

#include <string.h>
#include <time.h>

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#define TCP_SERVER_IP "192.168.1.217"
#define TCP_PORT 1030

#define SEND_BUFF_SIZE 3
#define RECV_BUFF_SIZE 64
#define DEBUG_printf printf
#define TEST_ITERATIONS 10
#define POLL_TIME_S 5

#if 0
static void dump_bytes(const uint8_t *bptr, uint32_t len) {
    unsigned int i = 0;

    printf("dump_bytes %d", len);
    for (i = 0; i < len;) {
        if ((i & 0x0f) == 0) {
            printf("\n");
        } else if ((i & 0x07) == 0) {
            printf(" ");
        }
        printf("%02x ", bptr[i++]);
    }
    printf("\n");
}
#define DUMP_BYTES dump_bytes
#else
#define DUMP_BYTES(A, B)
#endif

typedef struct TCP_CLIENT_T_ {
    struct tcp_pcb *tcp_pcb;
    ip_addr_t remote_addr;
    uint8_t sendBuff[SEND_BUFF_SIZE];
    uint8_t recvBuff[RECV_BUFF_SIZE];
    int recv_buffer_len;
    bool complete;
    bool connected;
} TCP_CLIENT_T;

static err_t tcp_client_close(void *arg) {
    TCP_CLIENT_T *state = (TCP_CLIENT_T *)arg;
    err_t err = ERR_OK;
    if (state->tcp_pcb != NULL) {
        tcp_arg(state->tcp_pcb, NULL);
        tcp_poll(state->tcp_pcb, NULL, 0);
        tcp_sent(state->tcp_pcb, NULL);
        tcp_recv(state->tcp_pcb, NULL);
        tcp_err(state->tcp_pcb, NULL);
        err = tcp_close(state->tcp_pcb);
        if (err != ERR_OK) {
            DEBUG_printf("close failed %d, calling abort\n", err);
            tcp_abort(state->tcp_pcb);
            err = ERR_ABRT;
        }
        state->tcp_pcb = NULL;
    }
    return err;
}

// Called with results of operation
static err_t tcp_result(void *arg, int status) {
    TCP_CLIENT_T *state = (TCP_CLIENT_T *)arg;
    if (status == 0) {
        DEBUG_printf("test success\n");
    } else {
        DEBUG_printf("test failed %d\n", status);
    }
    state->complete = true;
    return tcp_client_close(arg);
}

static err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    TCP_CLIENT_T *state = (TCP_CLIENT_T *)arg;
    DEBUG_printf("tcp_client_sent %u\n", len);
    return ERR_OK;
}

static err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err) {
    DEBUG_printf("Connected to the server\n");
    TCP_CLIENT_T *state = (TCP_CLIENT_T *)arg;
    if (err != ERR_OK) {
        printf("connect failed %d\n", err);
        return tcp_result(arg, err);
    }
    state->connected = true;

    //
    // We'll kick things off by writing a bit of data to the server.
    err = tcp_write(tpcb, state->sendBuff, SEND_BUFF_SIZE, TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        DEBUG_printf("Failed to write data %d\n", err);
        return tcp_result(arg, -1);
    }
    //
    // No sense waiting around here.
    err = tcp_output(tpcb);
    if (err != ERR_OK) {
        DEBUG_printf("Failed to output data %d\n", err);
        return tcp_result(arg, -1);
    }

    return ERR_OK;
}

static err_t tcp_client_poll(void *arg, struct tcp_pcb *tpcb) {
    DEBUG_printf("tcp_client_poll\n");
    return tcp_result(arg, -1);  // no response is an error?
}

static void tcp_client_err(void *arg, err_t err) {
    if (err != ERR_ABRT) {
        DEBUG_printf("tcp_client_err %d\n", err);
        tcp_result(arg, err);
    }
}

err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p,
                      err_t err) {
    TCP_CLIENT_T *state = (TCP_CLIENT_T *)arg;
    if (!p) {
        return tcp_result(arg, -1);
    }
    // this method is callback from lwIP, so cyw43_arch_lwip_begin is not
    // required, however you can use this method to cause an assertion in debug
    // mode, if this method is called when cyw43_arch_lwip_begin IS needed
    cyw43_arch_lwip_check();
    if (p->tot_len > 0) {
        DEBUG_printf("recv %d err %d\n", p->tot_len, err);
        for (struct pbuf *q = p; q != NULL; q = q->next) {
            DUMP_BYTES(q->payload, q->len);
        }
        // Receive the buffer
        const uint16_t buffer_left = RECV_BUFF_SIZE - state->recv_buffer_len;
        state->recv_buffer_len += pbuf_copy_partial(
            p, state->recvBuff + state->recv_buffer_len,
            p->tot_len > buffer_left ? buffer_left : p->tot_len, 0);
        tcp_recved(tpcb, p->tot_len);
    }
    pbuf_free(p);

    return ERR_OK;
}

static bool tcp_client_open(void *arg) {
    TCP_CLIENT_T *state = (TCP_CLIENT_T *)arg;
    DEBUG_printf("Connecting to %s port %u\n",
                 ip4addr_ntoa(&state->remote_addr), TCP_PORT);
    state->tcp_pcb = tcp_new_ip_type(IP_GET_TYPE(&state->remote_addr));
    if (!state->tcp_pcb) {
        DEBUG_printf("failed to create pcb\n");
        return false;
    }

    //
    // Registering callbacks for various events.
    tcp_arg(state->tcp_pcb, state);
    tcp_poll(state->tcp_pcb, tcp_client_poll, POLL_TIME_S * 2);
    tcp_sent(state->tcp_pcb, tcp_client_sent);
    tcp_recv(state->tcp_pcb, tcp_client_recv);
    tcp_err(state->tcp_pcb, tcp_client_err);

    state->recv_buffer_len = 0;

    // cyw43_arch_lwip_begin/end should be used around calls into lwIP to ensure
    // correct locking. You can omit them if you are in a callback from lwIP.
    // Note that when using pico_cyw_arch_poll these calls are a no-op and can
    // be omitted, but it is a good practice to use them in case you switch the
    // cyw43_arch type later.
    printf("Ready to connect\n");
    cyw43_arch_lwip_begin();
    err_t err = tcp_connect(state->tcp_pcb, &state->remote_addr, TCP_PORT,
                            tcp_client_connected);
    cyw43_arch_lwip_end();
    printf("Connected\n");

    return err == ERR_OK;
}

// Perform initialisation
static TCP_CLIENT_T *tcp_client_init(void) {
    TCP_CLIENT_T *state = (TCP_CLIENT_T *)calloc(1, sizeof(TCP_CLIENT_T));
    if (!state) {
        DEBUG_printf("failed to allocate state\n");
        return NULL;
    }
    //
    // Send "foo", in bytes.
    state->sendBuff[0] = 146;
    state->sendBuff[1] = 157;
    state->sendBuff[2] = 157;
    ip4addr_aton(TCP_SERVER_IP, &state->remote_addr);
    return state;
}

data_t *fetch_data() {
    TCP_CLIENT_T *state = tcp_client_init();
    if (!state) {
        return NULL;
    }
    if (!tcp_client_open(state)) {
        tcp_result(state, -1);
        return NULL;
    }

    while (!state->complete) {
        //
        // We're polling, as we need to do this before we do anything else.
        cyw43_arch_poll();
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(1000));
    }

    //
    // Parse the returned data into our data structure. I know this is gross.
    data_t *ret = (data_t *)calloc(1, sizeof(data_t));
    int el = 0;
    int ps = 0;
    for (int i = 0; i < state->recv_buffer_len && el < 11; i++) {
        if (state->recvBuff[i] == '\n') {
            //
            // Null terminate this substring.
            state->recvBuff[i] = 0;
            switch (el) {
                case 0:
                    ret->dt.year = atoi((const char *)&(state->recvBuff[ps]));
                    break;
                case 1:
                    ret->dt.month = atoi((const char *)&(state->recvBuff[ps]));
                    break;
                case 2:
                    ret->dt.day = atoi((const char *)&(state->recvBuff[ps]));
                    break;
                case 3:
                    ret->dt.hour = atoi((const char *)&(state->recvBuff[ps]));
                    break;
                case 4:
                    ret->dt.min = atoi((const char *)&(state->recvBuff[ps]));
                    break;
                case 5:
                    ret->dt.sec = atoi((const char *)&(state->recvBuff[ps]));
                    break;
                case 6:
                    memcpy(ret->sunrise, &state->recvBuff[ps], i - ps);
                    ret->sunrise[i - ps] = 0;
                    break;
                case 7:
                    memcpy(ret->sunset, &state->recvBuff[ps], i - ps);
                    ret->sunset[i - ps] = 0;
                    break;
                case 8:
                    ret->pop = atoi((const char *)&(state->recvBuff[ps]));
                    break;
                case 9:
                    ret->high = atoi((const char *)&(state->recvBuff[ps]));
                    break;
                case 10:
                    ret->low = atoi((const char *)&(state->recvBuff[ps]));
                    break;
            }
            ps = i + 1;
            el++;
        }
    }
    free(state);
    return ret;
}