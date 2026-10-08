// The desktop simulator for PicoLEDs. It provides the Pico SDK functions our
// stub headers declare (the clock, sleeping, random numbers), takes the frames
// the simulated Renderer produces, and either shows them in a window or
// records them to a GIF.
//
// The program being simulated is a normal Pico program: its main() gets
// renamed to picoleds_user_main() by the build, and we run it.

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <chrono>
#include <map>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include "Canvas.h"
#include "GifWriter.h"
#include "SimWindow.h"
#include "sim_hooks.h"

int picoleds_user_main();

namespace {

struct Options {
    std::string gif;
    double seconds = 10;
    int scale = 0;
    int wrap = 0;
    bool window = true;
    bool gamma = true;
    uint32_t seed = 1;
};

Options opts;

// --- The clock ---------------------------------------------------------------
//
// With a window we run in real time. When we're only recording a GIF we use a
// virtual clock that moves when the program sleeps or sends a frame, so a
// recording comes out the same every time and doesn't take as long as the
// animation does.

bool virtualTime = false;
bool recording = false;
uint64_t vnow = 0;
auto realStart = std::chrono::steady_clock::now();

void advance(uint64_t us);

// --- Frames ------------------------------------------------------------------

std::map<Renderer*, Canvas*> canvases;
std::mutex canvasLock;

struct Frame {
    int w = 0, h = 0;
    std::vector<uint8_t> rgb;
};

uint8_t gammaLUT[256];

/// @brief Draws a frame of LEDs as round dots on a black background, with the
/// bottom row of the canvas at the bottom of the image.
void rasterize(const Frame& f, int scale, std::vector<uint8_t>& out) {
    int W = f.w * scale, H = f.h * scale;
    out.assign((size_t)W * H * 3, 0);
    //
    // A mask for one LED, so a dot is a dot and not a square once it's big
    // enough to tell the difference.
    std::vector<uint8_t> mask(scale * scale, 255);
    if (scale >= 4) {
        float r = (scale - 1) / 2.0f;
        float c = (scale - 1) / 2.0f;
        for (int y = 0; y < scale; y++) {
            for (int x = 0; x < scale; x++) {
                float d = sqrtf((x - c) * (x - c) + (y - c) * (y - c));
                float a = r - d + 0.5f;
                mask[y * scale + x] = a >= 1 ? 255 : a <= 0 ? 0 : (uint8_t)(a * 255);
            }
        }
    }
    for (int y = 0; y < f.h; y++) {
        int top = (f.h - 1 - y) * scale;
        for (int x = 0; x < f.w; x++) {
            const uint8_t* c = &f.rgb[((size_t)y * f.w + x) * 3];
            uint8_t cc[3] = {gammaLUT[c[0]], gammaLUT[c[1]], gammaLUT[c[2]]};
            if (c[0] == 0 && c[1] == 0 && c[2] == 0) {
                //
                // Pixels that are off are a faint gray, so you can see where
                // the LEDs are.
                cc[0] = cc[1] = cc[2] = 18;
            }
            for (int dy = 0; dy < scale; dy++) {
                uint8_t* p = &out[(((size_t)top + dy) * W + x * scale) * 3];
                for (int dx = 0; dx < scale; dx++) {
                    int m = mask[dy * scale + dx];
                    for (int k = 0; k < 3; k++) {
                        *p++ = (cc[k] * m + 127) / 255;
                    }
                }
            }
        }
    }
}

int pickScale(int w, int h) {
    if (opts.scale > 0) {
        return opts.scale;
    }
    //
    // GIFs get kept a bit smaller so they're easy to pass around.
    int maxW = recording ? 720 : 1200;
    int maxH = recording ? 480 : 800;
    int s = std::min(maxW / std::max(w, 1), maxH / std::max(h, 1));
    return std::max(2, std::min(s, 16));
}

// --- Recording ---------------------------------------------------------------

GifWriter gif;
Frame pending;
uint64_t pendingStart = 0;
bool havePending = false;
double carryCS = 0;

void writePending(uint64_t until) {
    double cs = (until - pendingStart) / 10000.0 + carryCS;
    int delay = (int)lround(cs);
    carryCS = cs - delay;
    if (!gif.getFrameCount()) {
        int scale = pickScale(pending.w, pending.h);
        if (!gif.open(opts.gif, pending.w * scale, pending.h * scale)) {
            fprintf(stderr, "Can't write %s\n", opts.gif.c_str());
            exit(1);
        }
    }
    std::vector<uint8_t> img;
    rasterize(pending, pickScale(pending.w, pending.h), img);
    gif.addFrame(img.data(), delay);
}

void finishRecording() {
    recording = false;
    if (havePending) {
        writePending(vnow);
        havePending = false;
    }
    gif.close();
    printf("Wrote %d frames (%.1f s) to %s\n", gif.getFrameCount(),
           vnow / 1e6, opts.gif.c_str());
}

void record(const Frame& f) {
    uint64_t now = vnow;
    if (havePending) {
        if (f.w == pending.w && f.h == pending.h && f.rgb == pending.rgb) {
            //
            // Same picture as before (dithering re-sends frames), so it just
            // stays up longer.
            return;
        }
        //
        // Browsers show GIF frames shorter than 2/100 s for much longer than
        // asked, so anything that brief gets replaced by what comes next.
        if (now - pendingStart + carryCS * 10000 >= 20000 || f.w != pending.w ||
            f.h != pending.h) {
            writePending(now);
            pendingStart = now;
        }
    } else {
        pendingStart = now;
    }
    pending = f;
    havePending = true;
}

void advance(uint64_t us) {
    vnow += us;
    if (recording && vnow >= opts.seconds * 1e6) {
        finishRecording();
        fflush(stdout);
        //
        // The Pico program never returns, so this is where it ends. Skip the
        // static destructors, since the program's objects are still in use.
        _Exit(0);
    }
}

// --- The window --------------------------------------------------------------

std::mutex frameLock;
Frame latest;
uint64_t latestCount = 0;

void usage(const char* prog) {
    fprintf(stderr,
            "Usage: %s [options]\n"
            "  --gif FILE      record to an animated GIF instead of opening a window\n"
            "  --seconds N     how much of the animation to record (default 10)\n"
            "  --scale N       screen pixels per LED (default: fit the screen)\n"
            "  --wrap N        for programs without a Canvas, fold each strip into\n"
            "                  rows of N pixels (default 100 for strips over 150)\n"
            "  --seed N        random seed, for repeatable runs (default 1)\n"
            "  --no-gamma      show raw color values instead of how LEDs look\n",
            prog);
}

}  // namespace

// --- The Pico SDK functions the stubs declare ---------------------------------

uint64_t time_us_64(void) {
    if (virtualTime) {
        //
        // Programs that poll the clock waiting for time to pass would spin
        // forever on a clock that never moves, so every look costs a
        // microsecond.
        advance(1);
        return vnow;
    }
    return std::chrono::duration_cast<std::chrono::microseconds>(
               std::chrono::steady_clock::now() - realStart)
        .count();
}

void sleep_us(uint64_t us) {
    if (virtualTime) {
        advance(us);
    } else {
        SimWindow::waitWhilePaused();
        std::this_thread::sleep_for(std::chrono::microseconds(us));
    }
}

static std::mt19937 rng;

//
// colorutils.cpp's 2D blurs call an XY() that the program is supposed to
// provide. The Pico linker throws the blurs away when nothing uses them, but
// desktop linkers want XY() to exist anyway, so here's one a real definition
// will replace.
__attribute__((weak)) uint16_t XY(uint8_t x, uint8_t y) {
    fprintf(stderr, "blur2d needs an XY(x, y) function in your program\n");
    abort();
}

uint32_t get_rand_32(void) { return rng(); }

// --- The hooks the library calls -----------------------------------------------

void picoleds_sim_canvas_show(Canvas* canvas) {
    std::lock_guard<std::mutex> g(canvasLock);
    canvases[canvas->getRenderer()] = canvas;
}

Canvas* picoleds_sim_canvas_for(Renderer* renderer) {
    std::lock_guard<std::mutex> g(canvasLock);
    auto it = canvases.find(renderer);
    return it == canvases.end() ? nullptr : it->second;
}

void picoleds_sim_frame(int width, int height, const uint8_t* rgb,
                        uint64_t sendTimeUS, bool strips) {
    Frame f;
    int wrap = opts.wrap > 0 ? opts.wrap : width > 150 ? 100 : 0;
    if (strips && wrap > 0 && wrap < width) {
        //
        // A long strip on its own would be a thin line across the screen, so
        // fold each one into rows of wrap pixels, reading like a page: first
        // strip at the top, left to right, with a blank row between strips.
        int perStrip = (width + wrap - 1) / wrap;
        f.w = wrap;
        f.h = height * (perStrip + 1) - 1;
        f.rgb.assign((size_t)f.w * f.h * 3, 0);
        for (int s = 0; s < height; s++) {
            for (int x = 0; x < width; x++) {
                int line = s * (perStrip + 1) + x / wrap;
                int y = f.h - 1 - line;
                memcpy(&f.rgb[((size_t)y * f.w + x % wrap) * 3],
                       &rgb[((size_t)s * width + x) * 3], 3);
            }
        }
    } else {
        f.w = width;
        f.h = height;
        f.rgb.assign(rgb, rgb + width * height * 3);
    }
    if (recording) {
        record(f);
        advance(sendTimeUS);
    } else {
        {
            std::lock_guard<std::mutex> g(frameLock);
            latest = std::move(f);
            latestCount++;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(sendTimeUS));
    }
}

// Used by SimWindow to fetch the newest frame.
bool picoleds_sim_latest(uint64_t& seen, int& w, int& h, int& scale,
                         std::vector<uint8_t>& img) {
    Frame f;
    {
        std::lock_guard<std::mutex> g(frameLock);
        if (latestCount == seen) {
            return false;
        }
        seen = latestCount;
        f = latest;
    }
    if (f.w == 0 || f.h == 0) {
        return false;
    }
    scale = pickScale(f.w, f.h);
    w = f.w * scale;
    h = f.h * scale;
    rasterize(f, scale, img);
    return true;
}

int main(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        auto next = [&]() -> const char* {
            if (i + 1 >= argc) {
                usage(argv[0]);
                exit(2);
            }
            return argv[++i];
        };
        if (a == "--gif") {
            opts.gif = next();
        } else if (a == "--seconds") {
            opts.seconds = atof(next());
        } else if (a == "--scale") {
            opts.scale = atoi(next());
        } else if (a == "--wrap") {
            opts.wrap = atoi(next());
        } else if (a == "--seed") {
            opts.seed = strtoul(next(), nullptr, 10);
        } else if (a == "--no-gamma") {
            opts.gamma = false;
        } else {
            usage(argv[0]);
            return a == "--help" || a == "-h" ? 0 : 2;
        }
    }
    rng.seed(opts.seed);

    //
    // LEDs put out light in proportion to the value we send, but a screen
    // assumes values are gamma encoded, so dim colors look far dimmer on a
    // screen than on a strip. Undo that so the screen looks like the LEDs.
    for (int i = 0; i < 256; i++) {
        gammaLUT[i] = opts.gamma ? (uint8_t)lround(255 * pow(i / 255.0, 1 / 2.2)) : i;
    }

    if (!opts.gif.empty() || !SimWindow::available()) {
        if (opts.gif.empty()) {
            std::string name = argv[0];
            size_t slash = name.find_last_of('/');
            opts.gif = name.substr(slash == std::string::npos ? 0 : slash + 1) + ".gif";
            printf("Built without SDL2, so recording %.0f s to %s\n",
                   opts.seconds, opts.gif.c_str());
        }
        virtualTime = true;
        recording = true;
        picoleds_user_main();
        if (recording) {
            finishRecording();
        }
        return 0;
    }

    //
    // The window has to belong to the main thread (macOS insists), so the
    // Pico program runs on another one.
    std::thread user([]() {
        picoleds_user_main();
        SimWindow::programFinished();
    });
    user.detach();
    return SimWindow::run(argv[0]);
}
