#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>

#include "SimWindow.h"

namespace {

std::mutex pauseLock;
std::condition_variable unpaused;
bool paused = false;
std::atomic<bool> finished(false);

void setPaused(bool p) {
    std::lock_guard<std::mutex> g(pauseLock);
    paused = p;
    unpaused.notify_all();
}

}  // namespace

namespace SimWindow {

bool available() { return true; }

void waitWhilePaused() {
    std::unique_lock<std::mutex> g(pauseLock);
    unpaused.wait(g, [] { return !paused; });
}

void programFinished() { finished = true; }

int run(const char* path) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Can't start SDL: %s\n", SDL_GetError());
        return 1;
    }
    std::string title = path;
    size_t slash = title.find_last_of('/');
    if (slash != std::string::npos) {
        title = title.substr(slash + 1);
    }
    std::string baseTitle = "PicoLEDs: " + title;

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;
    int tw = 0, th = 0;
    uint64_t seen = 0;
    std::vector<uint8_t> img;

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
                        setPaused(!paused);
                        if (window != nullptr) {
                            SDL_SetWindowTitle(
                                window, (baseTitle + (paused ? " (paused)" : "")).c_str());
                        }
                        break;
                }
            }
        }

        int w, h, scale;
        if (picoleds_sim_latest(seen, w, h, scale, img)) {
            if (window == nullptr) {
                window = SDL_CreateWindow(baseTitle.c_str(), SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED, w, h,
                                          SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
                renderer = SDL_CreateRenderer(
                    window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
                if (renderer == nullptr) {
                    renderer = SDL_CreateRenderer(window, -1, 0);
                }
                SDL_RenderSetLogicalSize(renderer, w, h);
            }
            if (texture == nullptr || w != tw || h != th) {
                if (texture != nullptr) {
                    SDL_DestroyTexture(texture);
                }
                texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24,
                                            SDL_TEXTUREACCESS_STREAMING, w, h);
                tw = w;
                th = h;
                SDL_RenderSetLogicalSize(renderer, w, h);
            }
            SDL_UpdateTexture(texture, nullptr, img.data(), w * 3);
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
