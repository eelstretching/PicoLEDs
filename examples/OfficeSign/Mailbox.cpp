#include "Mailbox.h"

#include "pico/multicore.h"
#include "pico/time.h"

Mailbox::Mailbox() { mutex_init(&mutex); }

void Mailbox::publish(const SignMessage& msg) {
    mutex_enter_blocking(&mutex);
    latest = msg;
    hasUnread = true;
    mutex_exit(&mutex);

    multicore_fifo_push_blocking(kDoorbellValue);
}

bool Mailbox::take(SignMessage& out) {
    if (!multicore_fifo_rvalid()) {
        return false;
    }

    // Drain any doorbells that have piled up -- we only care that *something*
    // arrived, not how many times the FIFO was rung.
    while (multicore_fifo_rvalid()) {
        (void)multicore_fifo_pop_blocking();
    }

    bool got = false;
    mutex_enter_blocking(&mutex);
    if (hasUnread) {
        out = latest;
        hasUnread = false;
        got = true;
    }
    mutex_exit(&mutex);
    return got;
}

void Mailbox::setConnected(bool isConnected) {
    connected = isConnected;
    if (!isConnected) {
        disconnectedAtUs = time_us_64();
    }
}

uint64_t Mailbox::microsSinceDisconnected() const { return time_us_64() - disconnectedAtUs; }
