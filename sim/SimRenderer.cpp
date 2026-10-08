// A Renderer for the desktop simulator. It has the same interface as the real
// one in src/Renderer.cpp, but instead of setting up PIO programs and DMA,
// render() hands the frame to the simulator to be drawn in a window or written
// to a GIF.

#include <algorithm>

#include "Canvas.h"
#include "Renderer.h"
#include "sim_hooks.h"

/// @brief How long one pixel takes to clock out at 800 kHz: 24 bits at
/// 1.25 us each.
#define US_PER_PIXEL 30

PIOProgram::~PIOProgram() {}

Renderer::~Renderer() {
    for (uint8_t* e : ditherError) {
        delete[] e;
    }
}

void Renderer::add(Strip* strip) { strips.push_back(strip); }

void Renderer::setup() {
    if (dithering) {
        setupDithering();
    }
    setupDone = true;
}

void Renderer::setupDithering() {
    if (ditherError.size() == strips.size()) {
        return;
    }
    for (Strip* s : strips) {
        ditherError.push_back(new uint8_t[s->getNumPixels() * 3]());
    }
}

void Renderer::addPIOProgram(int startIndex, int startPin, int size) {}

void Renderer::render() {
    dw.start();
    renderCount++;

    //
    // The strips all go out in parallel, so the longest one decides how long
    // a frame takes to send.
    uint maxLen = 0;
    for (Strip* s : strips) {
        maxLen = std::max(maxLen, s->getNumPixels());
    }
    uint64_t sendTime = (uint64_t)maxLen * US_PER_PIXEL + RESET_TIME_US;

    //
    // We show the colors as they are on the canvas, before brightness
    // scaling, since a screen pixel at 32/255 looks a lot dimmer than an LED
    // does.
    std::vector<uint8_t> frame;
    int w, h;
    Canvas* canvas = picoleds_sim_canvas_for(this);
    if (canvas != nullptr) {
        w = canvas->getWidth();
        h = canvas->getHeight();
        frame.resize(w * h * 3);
        uint8_t* p = frame.data();
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                const RGB& c = canvas->get(x, y);
                *p++ = c.r;
                *p++ = c.g;
                *p++ = c.b;
            }
        }
    } else {
        //
        // No canvas, so each strip is a row, with the first strip at the
        // bottom.
        w = maxLen;
        h = strips.size();
        frame.assign(w * h * 3, 0);
        for (int y = 0; y < h; y++) {
            RGB* data = strips[y]->getData();
            uint8_t* p = frame.data() + y * w * 3;
            for (uint x = 0; x < strips[y]->getNumPixels(); x++) {
                *p++ = data[x].r;
                *p++ = data[x].g;
                *p++ = data[x].b;
            }
        }
    }
    picoleds_sim_frame(w, h, frame.data(), sendTime, canvas == nullptr);
    dw.finish();
}

uint32_t Renderer::getBlockedCount() { return 0; }

uint64_t Renderer::getDMATime() { return 0; }
