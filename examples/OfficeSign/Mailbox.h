#ifndef MAILBOX_H
#define MAILBOX_H

#pragma once

#include <atomic>

#include "SignMessage.h"
#include "pico/mutex.h"

/// @brief Inter-core "latest message wins" mailbox. Core 1 (BluetoothServer)
/// is the producer, core 0 (the animation loop) is the consumer.
///
/// The shared SignMessage is protected by a mutex; the multicore FIFO is used
/// purely as a doorbell to tell core 0 a new message is waiting -- the
/// message payload itself never travels over the FIFO.
///
/// Concurrency contract: this is latest-wins, not a queue. If two messages
/// are published before core 0 next calls take(), only the second is ever
/// seen -- the first is silently dropped. That's fine for this protocol:
/// every message type is either a one-shot mode switch or a data snapshot,
/// none need strict ordering/delivery guarantees against each other.
///
/// Also carries the sign's current BLE connection state -- separate from the
/// message-passing above, since it's continuously-updated status rather than
/// a discrete event. Plain atomics rather than the mutex: a single flag and
/// timestamp, read by ModeController as a soft/approximate check (see
/// ModeController::step()), not something that needs strict consistency.
class Mailbox {
   public:
    Mailbox();

    /// @brief Producer side (core 1). Publishes msg as the latest message,
    /// overwriting any not-yet-consumed one, then rings the FIFO doorbell.
    void publish(const SignMessage& msg);

    /// @brief Consumer side (core 0). Call once per frame. Non-blocking:
    /// drains the FIFO doorbell if any pushes are pending and, if there's an
    /// unread message, copies it into out and returns true.
    bool take(SignMessage& out);

    /// @brief Producer side (core 1). Records whether the sign currently has
    /// a laptop connected over BLE.
    void setConnected(bool isConnected);

    /// @brief Consumer side (core 0). Whether a laptop is currently connected.
    bool isConnected() const { return connected; }

    /// @brief Consumer side (core 0). How long the sign has been
    /// disconnected, in microseconds. Meaningless (but harmless to call)
    /// while isConnected() is true.
    uint64_t microsSinceDisconnected() const;

   private:
    mutex_t mutex;
    SignMessage latest;
    bool hasUnread = false;

    std::atomic<bool> connected{false};
    std::atomic<uint64_t> disconnectedAtUs{0};

    static constexpr uint32_t kDoorbellValue = 0xA5;
};

#endif
