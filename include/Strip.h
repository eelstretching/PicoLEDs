#ifndef STRIP_H
#define STRIP_H

#pragma once

#include "color.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/sem.h"
#include "pico/stdlib.h"
#include "pico/types.h"

#include "StopWatch.h"

/// @brief A struct to store a semaphore and a delay alarm that will allow us to
/// delay after sending data to the LEDs to give the strip time to actually
/// light up.
class StripResetDelay {
   public:
    alarm_id_t alarm = 0;
    semaphore sem;
};

/// @brief A static array of pointers to strip resets, one per DMA channel, as
/// the IRQ handler will have to reset alarms as nescesary for any of the
/// channels, since the IRQ handler is a global function, not a member one.
///
/// An element of this array will be non-null if we're doing DMA on that
/// channel.
static StripResetDelay *strip_delays[NUM_DMA_CHANNELS];

/*
 * RES time, specification says it needs at least 50 us, but 30 seems to work?
 */
#define RESET_TIME_US (55)

class Strip {
   protected:
    //
    // The pin this strip is on.
    uint pin;

    //
    // The PIO unit that it's using.
    PIO pio;

    //
    // The state machine in the PIO that we're using.
    int sm;

    //
    // The DMA channel we'll use for transfers to the strip.
    int dma_channel;

    //
    // The DMA buffer where we'll put our pixels for showing.
    uint32_t *dma_buff;

    //
    // A delay timer for this strip.
    StripResetDelay *delay;

    //
    // Stats for this strip.
    StopWatch *stats;

    //
    // The offset of the PIO program to run the strip.
    int offset;

    //
    // The bytes making up the data for this strip.
    RGB *data;

    //
    // The color order for this strip. Default is GRB.
    ColorOrder colorOrder = ColorOrder::ORGB;

    //
    // How many pixels there are in the strip. For now, we're going to assume
    // that strips are RGB.
    uint numPixels;

    //
    // We'll keep things dim by default.
    uint8_t fracBrightness = 0x20;

    //
    // Where the next pixel will be added.
    uint pos;

   public:
    Strip(uint pin, uint num_pixels);

    /// @brief Adds a pixel to this strip at the next position
    /// @param c The RGB color value to add for the pixel
    /// @return The position that the pixel was added.
    uint addPixel(RGB c);

    /// @brief Adds a pixel to this strip at the next position
    /// @param h The HSV color value to add for the pixel
    /// @return The position that the pixel was added.
    uint addPixel(HSV c);

    /// @brief Puts a color at a specific pixel in the strip.
    /// @param p The position where the pixel should be placed. If this is not
    /// within the bounds of the array, then no change will happen
    /// @param c The RGB color value to put in the strip.
    void putPixel(RGB c, uint p);

    /// @brief Puts n pixels from the given array into the strip, starting at
    /// position 0.
    /// @param pixels The data that we want to copy into our strip
    /// @param n The number of pixels to copy.
    void putPixels(RGB *pixels, uint n);

    /// @brief Puts n pixels from the given array into the strip, starting at
    /// position p.
    /// @param p The position in the strip where pixels should be copied.
    /// @param pixels The data that we want to copy into our strip
    /// @param n The number of pixels to copy.
    void putPixels(RGB *pixels, uint p, uint n);

    /// @brief Fills the strip with the given color.
    /// @param c The color to fill the strip with.
    void fill(RGB c);

    /// @brief Fills a range of the strip with a given color
    /// @param c The color to fill with
    /// @param start the index where to start
    /// @param n the number of pixels to fill
    void fill(RGB c, uint start, uint n);

    /// @brief Shows the strip, i.e., it sends the data out the PIO state
    /// machine.
    void show();

    uint getNumPixels() { return numPixels; }

    ColorOrder getColorOrder() { return colorOrder; }

    void setColorOrder(ColorOrder co) { colorOrder = co; }

    /// @brief Sets the fractional brightness for the whole strip.
    /// @param fractionalBrightness A value between 0 and 255
    /// that specifies the brightness level of the LEDs in the strip between 0
    /// (off) and 255 (all the way on).
    void setFractionalBrightness(uint8_t fractionalBrightness) {
        fracBrightness = fractionalBrightness;
    }

    uint8_t getFractionalBrightness() { return fracBrightness; }

    void reset() { pos = 0; }

    StopWatch getStripStats() { return *stats; }
};

#endif