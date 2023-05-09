#include "Strip.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hardware/dma.h"
#include "hardware/irq.h"
#include "ws2812.pio.h"

/// @brief An alarm function that gets called when the delay required at the end
/// of sending data to a strip has elapsed.
/// @param id The alarm that went off
/// @param user_data A pointer to a StripDelay that we can use to release the
/// semaphore associated with this channel, which will allow us to send data on
/// it again.
/// @return 0, as we do not wish the alarm to repeat.
static int64_t reset_delay_complete(alarm_id_t id, void* strip_delay) {
  //
  // Reset the alarm, and release the semaphore.
  ((StripResetDelay*)strip_delay)->alarm = 0;
  sem_release(&((StripResetDelay*)strip_delay)->sem);
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
    // if dma triggered for this channel and it's been used (has a CMinWait)
    if ((dma_hw->ints0 & (1 << i)) && strip_delays[i]) {
      dma_hw->ints0 = (1 << i);  // clear/ack IRQ
      /* safety check: is there somehow an alarm already running? */
      if (strip_delays[i]->alarm != 0) {
        cancel_alarm(strip_delays[i]->alarm); /* cancel it */
      }
      //
      // Set up alarm to wait for the required latch-in time for the LES at the
      // end of the transfer, which will release the associated semaphore.
      strip_delays[i]->alarm = add_alarm_in_us(
          RESET_TIME_US, reset_delay_complete, strip_delays[i], true);
      return;
    }
  }
}
static bool isr_installed = false;

Strip::Strip(uint pin, uint num_pixels) : pin(pin), numPixels(num_pixels) {
  data = new RGB[num_pixels];
  pos = 0;

  stats = new StripStats();
  delay = new StripResetDelay();

  //
  // Find an unclaimed PIO state machine and put the program there. We won't
  // panic if we can't find one, but not clear what we should do if we can't!
  const PIO pios[NUM_PIOS] = {pio0, pio1};
  // iterate over PIO instances
  for (unsigned int i = 0; i < NUM_PIOS; i++) {
    pio = pios[i];
    sm = pio_claim_unused_sm(pio, false);  // claim a state machine
    if (sm == -1) {
      continue;  // skip this PIO if no unused sm
    }
    offset = pio_add_program(pio, &ws2812_program);
    if (offset == -1) {
      pio_sm_unclaim(pio, sm);  // unclaim the state machine and skip this PIO
      continue;                 // if program couldn't be added
    }

    break;  // found pio and sm that work
  }

  if (offset == -1) return;  // couldn't find good pio and sm

  ws2812_program_init(pio, sm, offset, pin, 800000);

  //
  // Claim an unused DMA channel (there's 12 in total,, so this should also
  // usually work out fine)
  dma_channel = dma_claim_unused_channel(false);
  if (dma_channel == -1) return;  // no free DMA channel

  //
  // Set up a DMA Channel.
  //
  // Set up the semaphore that we'll use to decide when it's OK to send data the
  // next time. We'll leave it posted for the first time through so that we can
  // send data without delay.
  sem_init(&delay->sem, 1, 1);
  strip_delays[dma_channel] = delay;

  //
  // Now we'll need a buffer where we can put the pixels while they're being
  // DMA'd Most examples seem to do this, as I guess we can be modifying the
  // existing pixels while the last bunch are being non-blockingly DMAed to the
  // PIO block.
  dma_buff = (uint32_t*)malloc(numPixels * sizeof(uint32_t));

  dma_channel_config channel_config =
      dma_channel_get_default_config(dma_channel);
  channel_config_set_transfer_data_size(&channel_config, DMA_SIZE_32);
  channel_config_set_dreq(&channel_config, pio_get_dreq(pio, sm, true));
  dma_channel_configure(dma_channel, &channel_config, &pio->txf[sm], NULL, 1,
                        false);

  if (!isr_installed) {
    //
    // We only want to install the handler once, which we'll do the first time
    // that we configure a DMA channel.
    irq_add_shared_handler(DMA_IRQ_0, dma_complete_handler,
                           PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
    irq_set_enabled(DMA_IRQ_0, true);
    isr_installed = true;
  }
  dma_channel_set_irq0_enabled(dma_channel, true);
}

uint Strip::addPixel(RGB c) {
  uint p = pos;
  data[pos++] = c;
  return p;
}

uint Strip::addPixel(HSV c) {
  uint p = pos;
  hsv2rgb_raw(c, data[pos++]);
  return p;
}

void Strip::putPixel(uint p, RGB c) {
  if (p >= numPixels) {
    return;
  }
  data[p] = c;
}

void Strip::putPixels(RGB* pixels, uint n) { putPixels(pixels, 0, n); }

void Strip::putPixels(RGB* pixels, uint p, uint n) {
  if (p >= numPixels) {
    return;
  }
  if (p + n >= numPixels) {
    n = numPixels - p;
  }
  memcpy(&data[p], pixels, n * sizeof(RGB));
}

void Strip::fill(RGB c) {
  fill(c, 0, numPixels);
}

void Strip::fill(RGB c, uint start, uint n) {
  for(int i = start; i < start + n && i < numPixels; i++) {
    data[i] = c;
  }
}

void Strip::show() {
  uint64_t start = time_us_64();
  if (dma_channel != -1) {
    //
    // Put the data into the DMA buffer. Maybe there's a fancy memcpy-esque way
    // to do this?
    for (int i = 0; i < numPixels; i++) {
      dma_buff[i] = data[i].scale8(fracBrightness).getColor(colorOrder) << 8u;
    }
    //
    // Acquire the semaphore that we're using for this strip's delay, and then
    // start the DMA. The semaphore will be released in the interrupt handler.
    sem_acquire_blocking(&delay->sem);
    dma_channel_set_read_addr(dma_channel, (void*)dma_buff, false);
    dma_channel_set_trans_count(dma_channel, numPixels, true);
  } else {
    //
    // Send the pixels to the PIO state machine the old-fashioned way.
    for (int i = 0; i < numPixels; i++) {
      pio_sm_put_blocking(
          pio, sm, data[i].scale8(fracBrightness).getColor(colorOrder) << 8u);
    }
  }
  stats->showTime += (time_us_64() - start);
  stats->showCount++;
  pos = 0;
}

void StripStats::combine(StripStats other) {
  showCount += other.showCount;
  showTime += other.showCount;
}
