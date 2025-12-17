#ifndef RENDERER_H
#define RENDERER_H

#pragma once

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
 * RES time, specification says it needs at least 50 us, but some folks say it
 * can be as low as 9us!
 */
#define RESET_TIME_US (80)
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
    void *buffer;
    uint32_t buffSize;
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
static PIOProgram *pioPrograms[NUM_DMA_CHANNELS] = {0};

/// @brief A class for a thing that knows how to render a logical Strip to a
/// physical strip.
class Renderer {
   protected:
    //
    // The strips that we're being asked to render.
    std::vector<Strip> strips;

    StopWatch dw;

    bool setupDone = false;

    int renderCount = 0;

   public:
    /// @brief Construct a renderer for a single strip.
    /// @param pin The pin that the strip is connected to.
    /// @param strip The strip itself.
    Renderer();

    ~Renderer();

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
    void add(Strip &strip);

    /// @brief Renders the strips.
    void render(ColorMap *colorMap);

    /// @brief Gets the number of times that calls to render blocked on the
    /// semaphore.
    /// @return the number of times that calls blocked.
    uint32_t getBlockedCount();

    /// @brief Gets the total time spent on DMA activities, in microseconds.
    /// @return The total time spent on DMA
    uint64_t getDMATime();

    uint64_t getDataSetupTime() { return dw.totalTime; }
};
#endif