#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>

#include "SimWindow.h"

namespace {

// Running, paused, or single stepping. Pausing is just stepping with no steps
// asked for yet, so both stop the program at the same place: just before it
// shows a new frame.
std::mutex stepLock;
std::condition_variable stepped;
bool stepping = false;
int stepsLeft = 0;

std::atomic<bool> finished(false);

void setStepping(bool s) {
    std::lock_guard<std::mutex> g(stepLock);
    stepping = s;
    stepsLeft = 0;
    stepped.notify_all();
}

void step() {
    std::lock_guard<std::mutex> g(stepLock);
    if (!stepping) {
        //
        // The first press stops things where they are, so the next one
        // shows the very next frame.
        stepping = true;
        stepsLeft = 0;
    } else {
        stepsLeft++;
    }
    stepped.notify_all();
}

bool isStepping() {
    std::lock_guard<std::mutex> g(stepLock);
    return stepping;
}

}  // namespace

namespace SimWindow {

bool available() { return true; }

uint64_t waitToShow() {
    std::unique_lock<std::mutex> g(stepLock);
    if (!stepping || stepsLeft > 0) {
        if (stepping) {
            stepsLeft--;
        }
        return 0;
    }
    auto start = std::chrono::steady_clock::now();
    stepped.wait(g, [] { return !stepping || stepsLeft > 0; });
    if (stepping) {
        stepsLeft--;
    }
    return std::chrono::duration_cast<std::chrono::microseconds>(
               std::chrono::steady_clock::now() - start)
        .count();
}

void programFinished() { finished = true; }

int run(const char* path) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Can't start SDL: %s\n", SDL_GetError());
        return 1;
    }
    std::string name = path;
    size_t slash = name.find_last_of('/');
    if (slash != std::string::npos) {
        name = name.substr(slash + 1);
    }

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;
    int tw = 0, th = 0;
    uint64_t seen = 0;
    std::vector<uint8_t> img;
    SimView view = picoleds_sim_initial_view();
    bool redraw = false;
    std::string title;

    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            } else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE:
                    case SDLK_q:
                        running = false;
                        break;
                    case SDLK_SPACE:
                        setStepping(!isStepping());
                        break;
                    case SDLK_RIGHT:
                        step();
                        break;
                    case SDLK_v:
                        view = (SimView)((view + 1) % VIEW_COUNT);
                        redraw = true;
                        break;
                    case SDLK_1:
                    case SDLK_2:
                    case SDLK_3:
                        view = (SimView)(e.key.keysym.sym - SDLK_1);
                        redraw = true;
                        break;
                }
            }
        }

        int w, h;
        if (picoleds_sim_latest(seen, redraw, view, w, h, img)) {
            redraw = false;
            if (window == nullptr) {
                window = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED, w, h,
                                          SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
                renderer = SDL_CreateRenderer(
                    window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
                if (renderer == nullptr) {
                    renderer = SDL_CreateRenderer(window, -1, 0);
                }
            }
            if (texture == nullptr || w != tw || h != th) {
                if (texture != nullptr) {
                    SDL_DestroyTexture(texture);
                    //
                    // A new view has a new shape, so the window follows it.
                    SDL_SetWindowSize(window, w, h);
                }
                texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24,
                                            SDL_TEXTUREACCESS_STREAMING, w, h);
                tw = w;
                th = h;
                SDL_RenderSetLogicalSize(renderer, w, h);
            }
            SDL_UpdateTexture(texture, nullptr, img.data(), w * 3);
        }
        if (window != nullptr) {
            std::string t = "PicoLEDs: " + name + " (" + simViewNames[view] + ")";
            if (isStepping()) {
                t += " frame " + std::to_string(seen) +
                     ", stopped: right arrow steps, space runs";
            }
            if (t != title) {
                SDL_SetWindowTitle(window, t.c_str());
                title = t;
            }
        }
        if (renderer != nullptr) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            if (texture != nullptr) {
                SDL_RenderCopy(renderer, texture, nullptr, nullptr);
            }
            SDL_RenderPresent(renderer);
        }
        if (finished && window == nullptr) {
            running = false;
        }
        SDL_Delay(window == nullptr ? 5 : 1);
    }

    //
    // The Pico program never returns, so just leave.
    SDL_Quit();
    _Exit(0);
}

}  // namespace SimWindow
