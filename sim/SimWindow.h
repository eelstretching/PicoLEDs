// The simulator's window. There are two versions: SimWindowSDL.cpp when SDL2
// is installed, and SimWindowNone.cpp when it isn't (then we can only record
// GIFs).
#ifndef PICOLEDS_SIM_WINDOW_H
#define PICOLEDS_SIM_WINDOW_H

#include <stdint.h>

#include <vector>

/// @brief Fetches the newest frame, drawn as LEDs, if it's newer than seen.
bool picoleds_sim_latest(uint64_t& seen, int& w, int& h, int& scale,
                         std::vector<uint8_t>& img);

namespace SimWindow {

/// @brief Whether we can open a window at all.
bool available();

/// @brief Runs the window until it's closed. Must be called on the main
/// thread.
int run(const char* title);

/// @brief Blocks the calling (program) thread while the window is paused.
void waitWhilePaused();

/// @brief Tells the window that the Pico program's main() returned.
void programFinished();

}  // namespace SimWindow

#endif
