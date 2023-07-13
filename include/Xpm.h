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
    RGB *pixels;

   public:
    /// @brief Creates a pixmap
    /// @param xpm the definition of the pixmap.
    Xpm(const char *xpm[]);
    /// @brief render this pixmap onto a canvas
    /// @param canvas the canvas to render on
    /// @param x the x coordinate to render at
    /// @param y the y coordinate to render at
    void render(Canvas *canvas, uint x, uint y);
    uint8_t getHeight() {return h;};
    uint8_t getWidth() {return w;};
};
#endif