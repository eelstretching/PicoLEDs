#ifndef XPM_H
#define XPM_H

#pragma once

#include "Canvas.h"
#include "color.h"
#include "pico/stdlib.h"
#include "pico/types.h"

/// @brief A slightly modified Xpm format that uses color indices rather than the colors themselves.
class Xpm {
   protected:
    uint8_t nc;
    uint8_t h;
    uint8_t w;
    //
    // The color indices in this pixmap.
    uint8_t *colorIndexes;
    //
    // The pixel data, as indices into the color map.
    uint8_t *pixels;

   public:
    /// @brief Creates a pixmap
    /// @param xpm the definition of the pixmap.
    Xpm(const char *xpm[]);
    Xpm(const Xpm& other);
    /// @brief Replace on color index with another in the pixmap.
    /// @param oldColor 
    /// @param newColor 
    void replaceColor(uint8_t oldColor, uint8_t newColor);
    /// @brief render this pixmap onto a canvas
    /// @param canvas the canvas to render on
    /// @param x the x coordinate to render at
    /// @param y the y coordinate to render at
    /// @return true if any pixel of the pixmap was rendered onto the canvas,
    /// false otherwise. This will give us a way to detect if a pixmap was
    /// rendered "off-screen" when making sprites.
    bool render(Canvas *canvas, uint x, uint y);
    /// @brief Renders the pixmap with a different set of colors, which means we
    /// only need one set of bitmaps for several different colored characters
    /// (e.g., the Pac-Man ghosts).
    /// @param canvas The canvas to render on
    /// @param remap An array to remap the color indices into a more useful colorMap.
    /// @param x The x coordinate to render at
    /// @param y the y coordinate to render at
    bool render(Canvas *canvas, uint8_t *remap, uint x, uint y);
    uint8_t getNumberOfColors() { return nc; };
    uint8_t getHeight() { return h; };
    uint8_t getWidth() { return w; };
    void dump();
};
#endif