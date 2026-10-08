// The simulator's window. There are two versions: SimWindowSDL.cpp when SDL2
// is installed, and SimWindowNone.cpp when it isn't (then we can only record
// GIFs).
#ifndef PICOLEDS_SIM_WINDOW_H
#define PICOLEDS_SIM_WINDOW_H

#include <stdint.h>

#include <vector>

/// @brief The ways we can draw a frame.
enum SimView {
    /// @brief The canvas as the animation sees it: x to the right, y up.
    VIEW_FLAT,
    /// @brief Turned so x goes up, the way a MegaTree's strands run from the
    /// ground to the top, and y goes across, around the tree.
    VIEW_TREE,
    /// @brief Looking down on a MegaTree from above, with the street at the
    /// bottom: each canvas row is a strand, running from the outside (x = 0,
    /// the bottom of the tree) to the middle (the top).
    VIEW_RADIAL,
    /// @brief A MegaTree seen from the street: a cone, with each strand going
    /// from the base up to the top.
    VIEW_FRONT,
    VIEW_COUNT
};

extern const char* const simViewNames[VIEW_COUNT];

/// @brief The view the program was started with.
SimView picoleds_sim_initial_view();

/// @brief Fetches the newest frame, drawn in the given view, if it's newer
/// than seen or force is set.
/// @param seen the number of the last frame fetched, updated to this one.
/// @return false if there was nothing new to draw.
bool picoleds_sim_latest(uint64_t& seen, bool force, SimView view, int& w,
                         int& h, std::vector<uint8_t>& img);

namespace SimWindow {

/// @brief Whether we can open a window at all.
bool available();

/// @brief Runs the window until it's closed. Must be called on the main
/// thread.
int run(const char* title);

/// @brief Called by the program thread before it shows a new frame. Blocks
/// while the window is paused, or until the next step when single stepping.
/// @return how long we blocked, in microseconds, so the simulated clock can
/// leave that time out.
uint64_t waitToShow();

/// @brief Tells the window that the Pico program's main() returned.
void programFinished();

}  // namespace SimWindow

#endif
