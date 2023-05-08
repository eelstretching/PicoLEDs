#ifndef CANVAS_H
#define CANVAS_H

#pragma once

#include "color.h"

#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/sem.h"
#include "pico/stdlib.h"
#include "pico/types.h"

class View;

class Canvas {

    friend class View;

    //
    // The width and height of the canvas, in pixels.
    uint width;
    uint height;
    uint numPixels;

    //
    // Where the current view is organized.
    uint viewX = 0;
    uint viewY = 0;

    //
    // The actual data for the canvas. If you think of the canvas as a 2-D
    // object, the origin would be in the bottom-left.
    RGB *data;

    /// @brief A view that we can use to render some or all of this canvas.
    View *view = nullptr;

   public:
    Canvas(uint width, uint height);

    /// @brief Sets a pixel on this canvas to the given color
    /// @param x the x co-ordinate of the pixel
    /// @param y the y co-ordinate of the pixel
    /// @param p the color the pixel should be set to.
    void set(uint x, uint y, RGB p);

    /// @brief Fills a row with the given color.
    /// @param row the row to fill
    /// @param p the color
    void fill(uint row, RGB p);

    /// @brief Gets the position in our data array for a given x and y. Just here so friends can figure it out
    /// @param x the x co-ordinate
    /// @param y the y co-ordinate
    /// @return the index into our data array for this x and y.
    uint getPos(uint x, uint y);

    /// @brief Clears the canvas.
    void clear();

    /// @brief Sets the origin of the view in this canvas. This lets us make a
    /// larger canvas and shift things by moving the view.
    /// @param x the x position of the view origin
    /// @param y the y position of the view origin
    void setView(View *v, uint x, uint y);

    /// @brief Shows the data on the canvas in the view provided by the strips.
    void show();
};

#endif