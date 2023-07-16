#ifndef XPM_H
#define XPM_H

#pragma once

#include "Canvas.h"
#include "color.h"
#include "pico/stdlib.h"
#include "pico/types.h"

class Xpm {
   protected:
    uint8_t nc;
    uint8_t h;
    uint8_t w;
    RGB *colors;
    uint8_t *pixels;

   public:
    /// @brief Creates a pixmap
    /// @param xpm the definition of the pixmap.
    Xpm(const char *xpm[]);
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
    /// @param colorMap The colorMap to use. We're not going to do a lot of
    /// checking on this, so beware.
    /// @param x The x coordinate to render at
    /// @param y the y coordinate to render at
    bool render(Canvas *canvas, RGB *colorMap, uint x, uint y);
    RGB *getColors() { return colors; };
    uint8_t getNumberOfColors() { return nc; };
    uint8_t getHeight() { return h; };
    uint8_t getWidth() { return w; };
    void dump();
};
#endif