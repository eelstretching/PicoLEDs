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
    for (unsigned int i = 0; i < NUM_DMA_CHANNELS; i++) {
        // if dma triggered for this channel and it's been used (has a delay
        // associated with it)
        if ((dma_hw->ints0 & (1 << i)) && strip_delays[i]) {
            dma_hw->ints0 = (1 << i);  // clear/ack IRQ
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

PIOProgram::~PIOProgram() {
    pio_sm_set_enabled(pio, sm, false);
    pio_sm_unclaim(pio, sm);
    pio_remove_program(
        pio, size == 1 ? &ws2812_program : &ws2812_parallel_program, offset);
    dma_channel_abort(dma_channel);
    dma_channel_unclaim(dma_channel);
    if (buffer) {
        free(buffer);
    }
}

Renderer::Renderer() {}

Renderer::~Renderer() {
    for (auto pip : programs) {
        delete pip;
    }
    programs.clear();
    strips.clear();
    for (int i = 0; i < NUM_DMA_CHANNELS; i++) {
        if (strip_delays[i]) {
            strip_delays[i] = nullptr;
        }
    }
    if (isr_installed) {
        irq_set_enabled(DMA_IRQ_0, false);
        irq_remove_handler(DMA_IRQ_0, dma_complete_handler);
        isr_installed = false;
    }
}

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
        } else if (s.getPin() == startPin + pinCount && pinCount < 8) {
            //
            // Continuing a run, but no run longer than 8 pins.
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
    // we can send data without delay. Also, we'll put the strip delay into the
    // global array so that the interrupt service routine can find it.
    sem_init(&pip->delay.sem, 1, 1);
    strip_delays[pip->dma_channel] = &pip->delay;

    //
    // Now we'll need a buffer where we can put the pixels while they're being
    // DMA'd Most examples seem to do this, as I guess we can be modifying the
    // existing pixels while the last bunch are being non-blockingly DMAed to
    // the PIO block. We're naively assuming here that the strips all have the
    // same number of pixels, so we can just use the size of the first one.
    // Maybe better to pick the minimum length one?
    if (pip->size == 1) {
        pip->buffSize = strips[startIndex].getNumPixels();
        pip->buffer = calloc(pip->buffSize, sizeof(uint32_t));

        dma_channel_config channel_config =
            dma_channel_get_default_config(pip->dma_channel);
        channel_config_set_transfer_data_size(&channel_config, DMA_SIZE_32);
        channel_config_set_dreq(&channel_config,
                                pio_get_dreq(pip->pio, pip->sm, true));
        dma_channel_configure(pip->dma_channel, &channel_config,
                              &pip->pio->txf[pip->sm], NULL, 1, false);
    } else {
        //
        // Each pixel on the strip uses 24 bits of color, this array will have
        // ones where the strips have that bit of color.
        pip->buffSize = strips[startIndex].getNumPixels() * 24;
        pip->buffer = calloc(pip->buffSize, sizeof(uint8_t));

        dma_channel_config channel_config =
            dma_channel_get_default_config(pip->dma_channel);
        channel_config_set_transfer_data_size(&channel_config, DMA_SIZE_8);
        channel_config_set_dreq(&channel_config,
                                pio_get_dreq(pip->pio, pip->sm, true));
        dma_channel_configure(pip->dma_channel, &channel_config,
                              &pip->pio->txf[pip->sm], NULL, 1, false);
    }

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
        //
        // We need to fill the buffer that we'll DMA to the PIO program. How
        // we fill it depends on how many strips we're sending.
        //
        // We want to track how long it takes to set up the data for the DMA.
        dw.start();

        //
        // Let's start by zeroing the buffer.
        if (pip->size == 1) {
            memset(pip->buffer, 0, pip->buffSize * sizeof(uint32_t));
            //
            // One strip, just put the data in the buffer according to the color
            // order after scaling by the strip's brightness.
            Strip s = strips[pip->startIndex];
            RGB *data = s.getData();
            uint32_t *pb = (uint32_t *)pip->buffer;
            for (int i = 0; i < s.getNumPixels(); i++, pb++, data++) {
                //
                // Note that we're shifting by 8 here because the PIO program
                // will be pulling 24 bits and it wants those 24 bits in the
                // most significant place.
                *pb = (*data)
                          .scale8(s.getFractionalBrightness())
                          .getColor(s.getColorOrder())
                      << 8u;
            }
        } else {
            memset(pip->buffer, 0, pip->buffSize * sizeof(uint8_t));
            //
            // We'll need to turn the data for multiple strips into bit planes.
            for (int i = pip->startIndex, sn = 0;
                 i < pip->startIndex + pip->size; i++, sn++) {
                Strip s = strips[i];
                RGB *data = s.getData();
                uint32_t pp = 0;

                //
                // If there is a one bit at this position in the strips
                // array, we'll always be or'ing in the same bit, so let's
                // just make it now. But remember that we need to start from 0
                // for this program!
                uint8_t stripBit = 1 << (i - pip->startIndex);
                for (int j = 0; j < s.getNumPixels(); j++, pp += 24, data++) {
                    uint8_t *pipbuff = &((uint8_t *) pip->buffer)[pp];
                    //
                    // Transform the data for color order and brightness. Doing
                    // that old school pointer bumping rather than array
                    // accesses to try to cut down on the time spent prepping
                    // the data.
                    uint32_t val = data->scale8(s.getFractionalBrightness())
                                       .getColor(s.getColorOrder());
                    // uint32_t val = data->getColor(s.getColorOrder());

                    //
                    // Unrolling the inner loop to save some ops. There's
                    // probably some crazy Duff's Device way to do this, but
                    // this is simple and it cuts about a third of the time in
                    // this loop.
                    if (val & 0x00800000) {
                        pipbuff[0] |= stripBit;
                    }
                    if (val & 0x00400000) {
                        pipbuff[1] |= stripBit;
                    }
                    if (val & 0x00200000) {
                        pipbuff[2] |= stripBit;
                    }
                    if (val & 0x00100000) {
                        pipbuff[3] |= stripBit;
                    }
                    if (val & 0x00080000) {
                        pipbuff[4] |= stripBit;
                    }
                    if (val & 0x00040000) {
                        pipbuff[5] |= stripBit;
                    }
                    if (val & 0x00020000) {
                        pipbuff[6] |= stripBit;
                    }
                    if (val & 0x00010000) {
                        pipbuff[7] |= stripBit;
                    }
                    if (val & 0x00008000) {
                        pipbuff[8] |= stripBit;
                    }
                    if (val & 0x00004000) {
                        pipbuff[9] |= stripBit;
                    }
                    if (val & 0x00002000) {
                        pipbuff[10] |= stripBit;
                    }
                    if (val & 0x00001000) {
                        pipbuff[11] |= stripBit;
                    }
                    if (val & 0x00000800) {
                        pipbuff[12] |= stripBit;
                    }
                    if (val & 0x00000400) {
                        pipbuff[13] |= stripBit;
                    }
                    if (val & 0x00000200) {
                        pipbuff[14] |= stripBit;
                    }
                    if (val & 0x00000100) {
                        pipbuff[15] |= stripBit;
                    }
                    if (val & 0x00000080) {
                        pipbuff[16] |= stripBit;
                    }
                    if (val & 0x00000040) {
                        pipbuff[17] |= stripBit;
                    }
                    if (val & 0x00000020) {
                        pipbuff[18] |= stripBit;
                    }
                    if (val & 0x00000010) {
                        pipbuff[19] |= stripBit;
                    }
                    if (val & 0x00000008) {
                        pipbuff[20] |= stripBit;
                    }
                    if (val & 0x00000004) {
                        pipbuff[21] |= stripBit;
                    }
                    if (val & 0x00000002) {
                        pipbuff[22] |= stripBit;
                    }
                    if (val & 0x00000001) {
                        pipbuff[23] |= stripBit;
                    }
                }
            }
        }
        dw.finish();

        // for(int k = 0; k < pip->buffSize; k++) {
        //     if(k > 0 && k % 24 == 0) {
        //         printf("\n");
        //     }
        //     printf("%4d %2d %032b\n", k, k % 24, pip->buffer[k]);
        // }

        //
        // We'll keep track of the time for the DMA ops.
        pip->delay.dma_start = time_us_64();

        dma_channel_set_read_addr(pip->dma_channel, pip->buffer, false);
        dma_channel_set_trans_count(pip->dma_channel, pip->buffSize, true);

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
