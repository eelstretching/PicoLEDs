#ifndef CANVAS_H
#define CANVAS_H

#pragma once

#include "StopWatch.h"
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
    uint rowBytes;

    //
    // Where the current view is organized.
    uint viewX = 0;
    uint viewY = 0;

    //
    // The actual data for the canvas. If you think of the canvas as a 2-D
    // object, the origin would be in the bottom-left.
    RGB *data;

    //
    // A background color. When we blank something out, when data gets copied in
    // from hyperspace or whatever, then we use this background color to fill.
    // By default, it's black.
    RGB background = RGB::Black;

    /// @brief A view that we can use to render some or all of this canvas.
    View *view = nullptr;

    /// @brief stats for our show operations.
    StopWatch stats;

   public:
    Canvas(uint width, uint height);

    /// @brief Set the background color.
    /// @param b the color to use.
    void setBackground(RGB b);

    /// @brief Sets a pixel on this canvas to the given color
    /// @param x the x co-ordinate of the pixel
    /// @param y the y co-ordinate of the pixel
    /// @param p the color the pixel should be set to.
    void set(uint x, uint y, RGB p);

    /// @brief Fills a row with the given color.
    /// @param row the row to fill
    /// @param p the color
    void fill(uint row, RGB p);

    /// @brief Fill the canvas with a given color
    /// @param c the color to fill with.
    void fill(RGB c);

    /// @brief Scrolls the canvas up by one row, filling the empty row with
    /// the background color.
    void scrollUp();

    /// @brief Scrolls the canvas up by n rows, filling the empty rows with the
    /// given color.
    /// @param n the number of rolls to scroll up by.
    /// @param f The color to fill the emptied rows.
    void scrollUp(uint n, RGB f);

    /// @brief Scroll the canvas down by one row, filling in the empty row with
    /// the background color.
    void scrollDown();

    /// @brief Scrolls the canvas down by n rows, filling in the empty row with
    /// the given color.
    /// @param n
    /// @param f
    void scrollDown(int n, RGB f);

    /// @brief Copies data from row src into row dst
    /// @param src The source of data
    /// @param dst The destination of data
    void copyRow(uint src, uint dst);

    /// @brief Rotate the canvas up by one row, which will make the top row the
    /// bottom row.
    void rotateUp();

    /// @brief Rotate the canvas down by one row, which will make the bottom row
    /// the top row.
    void rotateDown();

    /// @brief Mirrors the left side of the canvas onto the right through the
    /// middle column.
    /// @details If there are an even number of columns, the left half is simply
    /// mirrored. If there is an odd number of columns, the first column will
    /// not be mirrored.
    void mirrorLeftToRight();

    /// @brief Mirrors the left side of the canvas onto the right, where the
    /// "mirror" is at column c in the canvas.
    /// @param c the column where the mirroring will happen.
    void mirrorLeftToRight(int c);

    /// @brief Mirrors the right side of the canvas onto the left through the middle column.
    void mirrorRightToLeft();

    /// @brief Mirrors the right side of the canvas onto the left, where we mirror on column c.
    /// @param c the column to mirror on.
    void mirrorRightToLeft(int c);

    /// @brief Gets the position in our data array for a given x and y. Just
    /// here so friends can figure it out
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

    void debugPrint();

    StopWatch *getStopWatch();
};

#endif