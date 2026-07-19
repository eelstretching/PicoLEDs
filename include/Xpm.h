#ifndef XPM_H
#define XPM_H

#pragma once

#include "Canvas.h"
#include "color.h"
#include "pico/stdlib.h"
#include "pico/types.h"

/// @brief A slightly modified Xpm format that uses color indices rather than
/// the colors themselves.
class Xpm {
   protected:
    uint8_t nc;
    uint8_t h;
    uint8_t w;
    //
    // The colors in this pixmap.
    RGB* colors;

    //
    // The index of the background color in the Xpm, so that we can put pixmaps
    // on different backgrounds and we don't always get black. We assume that
    // this will be indicated by a capital B
    uint8_t backgroundColorIndex;

    //
    // The pixel data, as indices into the color map.
    uint8_t* pixels;

   public:
    /// @brief Creates a pixmap
    /// @param xpm the definition of the pixmap.
    Xpm(const char* xpm[]);
    Xpm(const Xpm& other);

    /// @brief render this pixmap onto a canvas
    /// @param canvas the canvas to render on
    /// @param x the x coordinate to render at
    /// @param y the y coordinate to render at
    /// @return true if any pixel of the pixmap was rendered onto the canvas,
    /// false otherwise. This will give us a way to detect if a pixmap was
    /// rendered "off-screen" when making sprites.
    bool render(Canvas* canvas, uint x, uint y);
    /// @brief Renders the pixmap with a different set of colors, which means we
    /// only need one set of bitmaps for several different colored characters
    /// (e.g., the Pac-Man ghosts).
    /// @param canvas The canvas to render on
    /// @param remap A different index to color mapping to use for rendering.
    /// @param x The x coordinate to render at
    /// @param y the y coordinate to render at
    bool render(Canvas* canvas, RGB* altColors, uint x, uint y);
    RGB *getColors() { return colors; };
    uint8_t getNumberOfColors() { return nc; };
    uint8_t getHeight() { return h; };
    uint8_t getWidth() { return w; };
    void dump();
};
#endif