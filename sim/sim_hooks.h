// Hooks between the PicoLEDs library and the desktop simulator. The library
// only calls these when it's built with PICOLEDS_SIM defined; on the Pico none
// of this exists.
#ifndef PICOLEDS_SIM_HOOKS_H
#define PICOLEDS_SIM_HOOKS_H

#include <stdint.h>

class Canvas;
class Renderer;

/// @brief Called by Canvas::show() just before it renders, so the simulator
/// can show the canvas (in canvas coordinates) rather than the raw strips.
void picoleds_sim_canvas_show(Canvas* canvas);

/// @brief Gets the canvas that last showed itself through this renderer, or
/// nullptr if the renderer is being used on its own.
Canvas* picoleds_sim_canvas_for(Renderer* renderer);

/// @brief Hands a frame to the simulator to display or record.
/// @param width the width of the frame, in pixels
/// @param height the height of the frame, in pixels
/// @param rgb width * height RGB triples, with row 0 at the bottom
/// @param sendTimeUS how long it would take to send this frame to the strips,
/// which the simulator uses to move its clock along.
/// @param strips true if each row is a raw strip rather than a canvas row.
void picoleds_sim_frame(int width, int height, const uint8_t* rgb,
                        uint64_t sendTimeUS, bool strips);

#endif
