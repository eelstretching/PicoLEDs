#include "Renderer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>

#include "hardware/dma.h"
#include "hardware/irq.h"
#include "pico/mem_ops.h"
#include "ws2812.pio.h"

/// @brief A comparator that we can use to sort strips by their pin number.
/// @param s1 The first strip
/// @param s2 The second strip
/// @return true if the first strip's pin number is lower than the second's.
bool pinComparator(Strip s1, Strip s2) { return s1.getPin() < s2.getPin(); }

/// @brief An alarm function that gets called when the reset delay required at
/// the end of sending data to a strip has elapsed.
/// @param id The alarm that went off
/// @param user_data A pointer to a StripDelay that we can use to release the
/// semaphore associated with this channel, which will allow us to send data on
/// it again.
/// @return 0, as we do not wish the alarm to repeat.
static int64_t reset_delay_complete(alarm_id_t id, void *strip_delay) {
    StripResetDelay *srd = (StripResetDelay *)strip_delay;

    reset_timer_expired++;

    //
    // Reset the alarm, and release the semaphore.
    srd->alarm = 0;
    sem_release(&srd->sem);
    srd->dma_time += (time_us_64() - srd->dma_start);
    return 0;
}

/// @brief An IRQ handler for whatever strips are sending data via DMA. Most
/// examples have a single strip, which is easier. We'll allow for multiple DMA
/// channels in use, which means the handler here will handle any of the defined
/// DMA channels that have completed their transfer and raised an interrupt. The
/// main thing here is to make sure that the delay necessary at the end of
/// sending data has elapsed.
/// @return nothing
static inline void __isr dma_complete_handler() {
    dma_irq_count++;

    for (unsigned int i = 0; i < NUM_DMA_CHANNELS; i++) {
        // if dma triggered for this channel and it's been used (has a delay
        // associated with it)
        if ((dma_hw->ints0 & (1 << i)) && strip_delays[i]) {
            dma_hw->ints0 = (1 << i);  // clear/ack IRQ
            dma_counts[i]++;
            /* safety check: is there somehow an alarm already running? */
            if (strip_delays[i]->alarm != 0) {
                cancel_alarm(strip_delays[i]->alarm); /* cancel it */
            }
            //
            // Set up alarm to wait for the required latch-in time for the LEDs
            // at the end of the transfer, which will release the associated
            // semaphore.
            strip_delays[i]->alarm = add_alarm_in_us(
                RESET_TIME_US, reset_delay_complete, strip_delays[i], true);
            return;
        }
    }
}

//
// A global letting us know that we've installed the interrupt service routine.
static bool isr_installed = false;

Renderer::Renderer() {}

void Renderer::add(Strip &strip) { strips.push_back(strip); }

void Renderer::setup() {
    if (setupDone) {
        return;
    }

    //
    // We have all our strips, lets break them into contiguous runs of pins
    // so that we can do as much in parallel as possible. We'll start by
    // sorting.
    std::sort(strips.begin(), strips.end(), pinComparator);

    int startIndex = -1;
    int startPin = -1;
    int pinCount = -1;
    for (int i = 0; i < strips.size(); i++) {
        Strip s = strips[i];
        if (startPin < 0) {
            //
            // Starting a new run.
            startIndex = i;
            startPin = s.getPin();
            pinCount = 1;
        } else if (s.getPin() == startPin + pinCount) {
            //
            // Continuing a run.
            pinCount++;
        } else {
            //
            // End of a run, generate a PIO program for this run of pins.
            addPIOProgram(startIndex, startPin, pinCount);
            startIndex = i;
            startPin = s.getPin();
            pinCount = 1;
        }
    }
    //
    // We have the last one we were counting to add.
    addPIOProgram(startIndex, startPin, pinCount);
    setupDone = true;
}

void Renderer::addPIOProgram(int startIndex, int startPin, int pinCount) {
    PIOProgram *pip = new PIOProgram();
    pip->startIndex = startIndex;
    pip->startPin = startPin;
    pip->size = pinCount;

    // This will find a free pio and state machine for our program (whether
    // serial or parallel) and load it for us.
    //
    // We use
    // pio_claim_free_sm_and_add_program_for_gpio_range (for_gpio_range variant)
    // so we will get a PIO instance suitable for addressing gpios >= 32 if
    // needed and supported by the hardware
    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(
        pip->size == 1 ? &ws2812_program : &ws2812_parallel_program, &pip->pio,
        &pip->sm, &pip->offset, startPin, pip->size, true);
    hard_assert(success);

    if (pip->size == 1) {
        ws2812_program_init(pip->pio, pip->sm, pip->offset, pip->startPin,
                            800000);
    } else {
        ws2812_parallel_program_init(pip->pio, pip->sm, pip->offset,
                                     pip->startPin, pip->size, 800000);
    }

    //
    // Claim an unused DMA channel (there's 12 in total,, so this should also
    // usually work out fine)
    pip->dma_channel = dma_claim_unused_channel(false);

    //
    // Flip if there's no DMA channel
    hard_assert(pip->dma_channel != -1);

    //
    // Set up a DMA Channel.
    //
    // Set up the semaphore that we'll use to decide when it's OK to send data
    // the next time. We'll leave it posted for the first time through so that
    // we can send data without delay.
    sem_init(&pip->delay.sem, 1, 1);
    strip_delays[pip->dma_channel] = &pip->delay;

    //
    // Now we'll need a buffer where we can put the pixels while they're being
    // DMA'd Most examples seem to do this, as I guess we can be modifying the
    // existing pixels while the last bunch are being non-blockingly DMAed to
    // the PIO block.
    if (pip->size == 1) {
        pip->buffSize = strips[startIndex].getNumPixels();
        pip->buffer = (uint32_t *)calloc(pip->buffSize, sizeof(uint32_t));
    } else {
        pip->buffSize = strips[startIndex].getNumPixels() * 24;
        pip->buffer = (uint32_t *)calloc(pip->buffSize, sizeof(uint32_t));
    }

    dma_channel_config channel_config =
        dma_channel_get_default_config(pip->dma_channel);
    channel_config_set_transfer_data_size(&channel_config, DMA_SIZE_32);
    channel_config_set_dreq(&channel_config,
                            pio_get_dreq(pip->pio, pip->sm, true));
    dma_channel_configure(pip->dma_channel, &channel_config,
                          &pip->pio->txf[pip->sm], NULL, 1, false);

    if (!isr_installed) {
        //
        // We only want to install the handler once, which we'll do the first
        // time that we configure a DMA channel.
        irq_add_shared_handler(DMA_IRQ_0, dma_complete_handler,
                               PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
        irq_set_enabled(DMA_IRQ_0, true);
        isr_installed = true;
    }
    dma_channel_set_irq0_enabled(pip->dma_channel, true);

    //
    // Looks like we're all set, so add this program to the list of ones that
    // we'll run.
    programs.push_back(pip);
}

void Renderer::render() {
    if (!setupDone) {
        setup();
    }
    for (auto pip : programs) {
        pip->stats.start();
        if (sem_available(&pip->delay.sem) == 0) {
            pip->nblocked++;
        }
        sem_acquire_blocking(&pip->delay.sem);
        pip->delay.dma_start = time_us_64();
        //
        // We need to fill the buffer that we'll DMA to the PIO program. How
        // we fill it depends on how many strips we're sending.
        if (pip->size == 1) {
            //
            // One strip, just put the data in the buffer according to the color
            // order after scaling by the strip's brightness.
            Strip s = strips[pip->startIndex];
            RGB *data = s.getData();
            dw.start();
            for (int i = 0; i < s.getNumPixels(); i++) {
                //
                // Note that we're shifting by 8 here because the PIO program
                // will be pulling 24 bits and it wants those 24 bits in the
                // most significant place.
                pip->buffer[i] = data[i]
                                     .scale8(s.getFractionalBrightness())
                                     .getColor(s.getColorOrder())
                                 << 8u;
            }
            dw.finish();
        } else {
            //
            // We'll need to turn the data for multiple strips into bit planes.
            dw.start();
            for (int i = pip->startIndex; i < pip->startIndex + pip->size;
                 i++) {
                Strip s = strips[i];
                RGB *data = s.getData();
                uint32_t pp = 0;

                //
                // If there is a one bit at this position in the strips
                // array, we'll always be or'ing in the same bit, so let's
                // just make it now.
                uint32_t aBit = 1 << i;
                for (int j = 0; j < s.getNumPixels(); j++) {
                    //
                    // Transform the data for color order and brightness.
                    uint32_t val = data[j]
                                       .scale8(s.getFractionalBrightness())
                                       .getColor(s.getColorOrder())
                                   << 8u;
                    //
                    // Plane it into the buffers.
                    for (int k = 0; k < 24; k++, pp++, val <<= 1) {
                        //
                        // We want the bits in most-to-least significant order.
                        if (val & 0x80000000) {
                            pip->buffer[pp] |= aBit;
                        }
                    }
                }
            }
            dw.finish();
        }
        //
        // OK, start the DMA. The semaphore will be released by the interrupt
        // service routine.
        dma_channel_set_read_addr(pip->dma_channel, (void *)pip->buffer, false);
        dma_channel_set_trans_count(pip->dma_channel, pip->buffSize, true);
        // printf("Start PIO\n");
        // for (int i = 0; i < pip->buffSize; i++) {
        //     // if (i % 24 == 0) {
        //     //     printf("\n");
        //     // }
        //     // printf("%04d %2d 0x%032b\n", i, i % 8, pip->buffer[i]);
        //     pio_sm_put_blocking(pip->pio, pip->sm, pip->buffer[i]);
        // }
        // sleep_us(RESET_TIME_US);

        pip->stats.finish();
    }
}

uint32_t Renderer::getBlockedCount() {
    uint32_t nb = 0;
    for (auto pip : programs) {
        nb += pip->nblocked;
    }
    return nb;
}

uint64_t Renderer::getDMATime() {
    uint64_t dt = 0;
    for (auto pip : programs) {
        dt += pip->delay.dma_time;
    }
    return dt;
}
