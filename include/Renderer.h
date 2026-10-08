#ifndef RENDERER_H
#define RENDERER_H

#pragma once

#include <functional>
#include <vector>

#include "StopWatch.h"
#include "Strip.h"
#include "color.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/sem.h"
#include "pico/stdlib.h"
#include "pico/types.h"

/*
 * RES time. The original WS2812 and WS2811 datasheets say at least 50 us, but
 * newer parts (WS2812B-V5 and friends) want 280 us or more. We also start
 * timing when the DMA finishes, which is up to 8 bits (about 10 us) before the
 * PIO program has actually shifted the last of the data out of its FIFO.
 */
#define RESET_TIME_US (300)
#define NUM_PARALLEL_PINS 8

class Renderer;

/// @brief A structure holding the details of a single PIO program for rendering
/// one or more strips.
class PIOProgram {
   public:
    ~PIOProgram();
    Renderer* renderer;
    pio_program_t* pio_program;
    PIO pio;
    uint sm;
    uint offset;
    int dma_channel;
    void* buffer;
    //
    // The number of elements in the buffer: pixels for a serial program,
    // bit-plane bytes for a parallel one.
    uint32_t buffSize;
    //
    // The number of DMA transfers needed to send the buffer. Not the same as
    // buffSize for a parallel program, where we DMA four bit-planes per word.
    uint32_t dmaCount;
    StopWatch stats;
    uint32_t nblocked = 0;
    int startIndex = 0;
    int startPin = 0;
    int size = 0;
    uint64_t dma_start = 0;
    uint64_t dma_time = 0;
    alarm_id_t alarm = 0;
    semaphore sem;
};

/// @brief A static array of pointers to PIO programs, one per DMA channel, as
/// the IRQ handler will have to reset alarms as nescesary for any of the
/// channels, since the IRQ handler is a global function, not a member one.
///
/// An element of this array will be non-null if we're running a program on that
/// DMA channel.
///
/// Make sure it's initialized to zeros, as we're counting on being able to test
/// which DMA channels need management.
extern PIOProgram* pioPrograms[NUM_DMA_CHANNELS];

/// @brief A class for a thing that knows how to render a logical Strip to a
/// physical strip.
class Renderer {
   protected:
    //
    // The strips that we're being asked to render.
    std::vector<Strip*> strips;

    StopWatch dw;

    bool setupDone = false;

    int renderCount = 0;

    // @brief Global brightness level for all strips we're rendering.
    uint8_t brightness;

    // @brief Whether we're doing temporal dithering when scaling by brightness.
    bool dithering = false;

    // @brief For each strip (in the same order as strips), the per-pixel,
    // per-channel remainder left over from the last brightness scaling. Only
    // allocated when dithering is on.
    std::vector<uint8_t*> ditherError;

    /// @brief Allocates the dither remainders for our strips, if we haven't
    /// already.
    void setupDithering();

   public:
    /// @brief Construct a renderer. We'll use a small default brightness
    /// because power and stuff.
    Renderer(uint8_t brightness = 32) : brightness(brightness) {}

    ~Renderer();

    void setBrightness(uint8_t brightness) { this->brightness = brightness; };

    uint8_t getBrightness() { return this->brightness; };

    /// @brief Turns temporal dithering on or off.
    ///
    /// Scaling by a low brightness throws away the low bits of each color
    /// channel, so at brightness 32 there are only about 33 levels per
    /// channel. With dithering on, the bits that get thrown away are kept per
    /// pixel and added back in on the next render, so a pixel that "should"
    /// be 4.5 alternates between 4 and 5 and averages out to 4.5. This only
    /// helps if render() is called well above the flicker rate (the Animator
    /// will re-render between frames when dithering is on).
    void setDithering(bool dithering) { this->dithering = dithering; };

    bool getDithering() { return dithering; };

    // @brief Processes a pixel color based on the current brightness, returing
    // the uint32 that we need to output to the strip. This is the default
    // function that we'll use if we don't call render with something else.
    virtual uint32_t processPixel(const RGB& color, Strip* strip) {
        return (uint32_t)color.scale8(brightness).getColor(strip->getColorOrder());
    };

    // @brief Processes a pixel color based on the current brightness, carrying
    // the remainder of the scaling over to the next render in err (3 bytes, one
    // per channel). This is what we'll use when dithering is on.
    virtual uint32_t processPixel(const RGB& color, Strip* strip, uint8_t* err) {
        RGB out(ditherChannel(color.r, err[0]), ditherChannel(color.g, err[1]),
                ditherChannel(color.b, err[2]));
        return (uint32_t)out.getColor(strip->getColorOrder());
    };

    // @brief Scales one channel by brightness the same way as scale8, but adds
    // in the remainder from last time and keeps the new remainder. Black stays
    // black and leaves the remainder alone so the pixel keeps its phase.
    inline uint8_t ditherChannel(uint8_t c, uint8_t& err) {
        if (c == 0) {
            return 0;
        }
        uint16_t v = (uint16_t)c * (1 + (uint16_t)brightness) + err;
        err = v & 0xFF;
        return v >> 8;
    }

    /// @brief After all strips have been added, set up for
    /// rendering by setting up PIO programs, DMA, etc.
    void setup();

    /// @brief Generate a PIO program and the associated stuff needed (DMA,
    /// buffers, etc.) for a run of pins
    /// @param startIndex the index in our strips where the run starts
    /// @param startPin the pin that starts the run
    /// @param pinCount the size of the run.
    void addPIOProgram(int startIndex, int startPin, int size);

    /// @brief Adds a strip to be rendered by this renderer.
    /// @param strip The strip to render
    void add(Strip* strip);

    /// @brief Renders the strips.
    void render();

    /// @brief Gets the number of times that calls to render blocked on the
    /// semaphore.
    /// @return the number of times that calls blocked.
    uint32_t getBlockedCount();

    /// @brief Gets the total time spent on DMA activities, in microseconds.
    /// @return The total time spent on DMA
    uint64_t getDMATime();

    uint64_t getAverageDataSetupTime() { return dw.getAverageTime(); }
};
#endif