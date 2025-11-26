#ifndef CANVAS_H
#define CANVAS_H

#pragma once

#include <vector>

#include "Renderer.h"
#include "StopWatch.h"
#include "Strip.h"
#include "color.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/sem.h"
#include "pico/stdlib.h"
#include "pico/types.h"

// @brief The direction that a strip goes, given the orientation of the canvas,
// as a single strip might have multiple rows in it.
enum StripDirection {
    FORWARDS,
    BACKWARDS,
};

class Canvas;

//
// A single row in this canvas, which is all or part of a given strip.
class Row {
    friend class Canvas;
    Canvas* canvas;
    Strip* strip;
    uint8_t *data;
    uint start;
    uint width;
    StripDirection dir;

   public:
    Row(Strip* strip, uint start, uint width, StripDirection dir,
        Canvas* canvas)
        : strip(strip), data(&strip->getData()[start]), start(start), width(width), dir(dir), canvas(canvas) {}

    /// @brief Sets the pixel at position x in this row to the given color.
    /// @param x the position of the pixel to set. This is relative to the
    /// origin of the canvas holding this row. This value will be translatd to
    /// the appropriate postion in the strip holding this row.
    /// @param color the color to set the pixel to.
    void set(int x, uint8_t color);

    /// @brief Gets the value of a pixel at position x in this row.
    /// @param x the position of the pixel relative to the origin of the canvas.
    /// @return the color of the pixel at the given position.
    uint8_t get(int x);

    /// @brief Fills a row with the given color
    /// @param color the color to fill with
    void fill(uint8_t color);

    /// @brief Rotate the row to the right by one pixel.
    void rotateRight();

    /// @brief Rotate the row to the left by one pixel.
    void rotateLeft();

    /// @brief Copies as many color values from the provided array at the given
    /// position.
    /// @param source The array containing values to be copied
    /// @param p The position at which copying should begin in our row
    /// @param n How many bytes we want to copy in total
    /// @return the number of bytes that were copied into this row, before we
    /// hit the end.
    int copy(uint8_t* source, int p, int n);

    /// @brief Copies the data from another row into this one.
    /// @param source the row whose data we should copy.
    void copy(Row* source);
};

// A 2-D canvas that we can draw on. If you think of the canvas as a 2-D
// object, the origin would be in the bottom-left.
class Canvas {
    //
    // The width of the canvas, in pixels, and a couple of associated values.
    uint width;
    uint numPixels;
    uint rowBytes;

    //
    // The color map that we'll use for this canvas. We could do one per strip,
    // but that would be really weird, so for now, it's one per canvas.
    ColorMap* colorMap;

    Renderer renderer;

    /// @brief The rows that make up this view, which are distributed across the
    /// strips that were added.
    Row **rows;

    uint8_t rowsLen = 0;
    uint8_t nRows = 0;

    //
    // The background color in the color map. When we blank
    // something out, when data gets copied in from hyperspace or whatever, then
    // we use this background color to fill. By default, it's black.
    RGB background = RGB(0, 0, 0);

    /// @brief stats for our show operations.
    StopWatch stats;

   public:
    Canvas(uint width);

    ~Canvas();

    void setColorMap(ColorMap* colorMap) { this->colorMap = colorMap; };

    ColorMap* getColorMap() { return colorMap; };

    ColorMap* makeColorMap(uint8_t size);

    void addRow(Row * row);

    /// @brief Adds a strip of pixels to this view.
    /// @param strip
    void add(Strip& strip);

    /// @brief Set up the canvas for operations.
    void setup() { renderer.setup(); };

    uint getHeight() { return nRows; };

    uint getWidth() { return width; };

    /// @brief Set the background color.
    /// @param b the color to use.
    void setBackground(RGB& background);

    RGB getBackground() { return colorMap->getBackground(); };

    inline uint8_t getBackgroundIndex() { return 255; };

    Row* getRow(int y) { return rows[y]; }

    /// @brief Sets a pixel on this canvas to the given color
    /// @param x the x co-ordinate of the pixel
    /// @param y the y co-ordinate of the pixel
    /// @param p the color the pixel should be set to.
    /// @return true, if it was possible to set this pixel, false if the pixel
    /// was not on the canvas as it was out-of-bounds.
    bool set(int x, int y, uint8_t p);

    /// @brief Gets the value of the pixel at the given coordinates, as an index
    /// into the color map.
    uint8_t get(uint x, uint y);

    /// @brief Copy the given data into the canvas, starting at position x,y.
    /// @param d the array of data to copy from
    /// @param n the number of pixels to copy in
    /// @param x the x coordinate where we want to do the copying.
    /// @param y the y coordinate where we want to do the copying.
    void copy(uint8_t* d, int n, int x, int y);

    /// @brief Fills a row with the given color.
    /// @param row the row to fill
    /// @param p the color
    void fillRow(uint row, uint8_t p);

    /// @brief Fills a column with the given color.
    /// @param col the column to fill
    /// @param p the color to fill with.
    void fillColumn(uint col, uint8_t p);

    /// @brief Fill the canvas with a given color
    /// @param c the color to fill with.
    void fill(uint8_t c);

    /// @brief Fills a rectangle with the given color.
    /// @param x0 The x coordinate of the bottom-left corner of the rectangle.
    /// @param y0 The y coordinate of the bottom-left corner of the rectangle.
    /// @param x1 The x coordinate of the top-right corner of the rectangle.
    /// @param y1 The y coordinate of the top-right corner of the rectangle.
    /// @param c The color to fill with.
    void fillRect(uint x0, uint y0, uint x1, uint y1, uint8_t c);

    /// @brief Draws a line from (x0,y0) to (x1,y1), using Bresenham's
    /// algorithm.
    /// @param x0 The x coordinate of the first point.
    /// @param y0 The y coordinate of the first point.
    /// @param x1 The x coordinate of the second point.
    /// @param y1 The y coordinate of the second point.
    /// @param c The color the line should be.
    void drawLine(uint x0, uint y0, uint x1, uint y1, uint8_t c);

    /// @brief Draws a line from (x0,y0) to (x1,y1), using Bresenham's
    /// algorithm, with optional wrap-around.
    /// @param x0 The x coordinate of the first point.
    /// @param y0 The y coordinate of the first point.
    /// @param x1 The x coordinate of the second point.
    /// @param y1 The y coordinate of the second point.
    /// @param c The color the line should be.
    /// @param wrapAround if true, the line will wrap around the canvas edges.
    void drawLine(uint x0, uint y0, uint x1, uint y1, uint8_t c,
                  bool wrapAround);

    /// @brief Draws a rectangle with diagonal corners (x0,y0) and (x1, y1)
    /// @param x0 The x coordinate of one corner
    /// @param y0 The y coordinate of one corner
    /// @param x1 The x coordinate of the other corner
    /// @param y1 The y coordinate of the other corner
    /// @param c The color of the line to draw with.
    void drawRect(uint x0, uint y0, uint x1, uint y1, uint8_t c);

    /// @brief Draws a rectangle with diagonal corners (x0,y0) and (x1, y1),
    /// filled with the given color.
    /// @param x0 The x coordinate of one corner
    /// @param y0 The y coordinate of one corner
    /// @param x1 The x coordinate of the other corner
    /// @param y1 The y coordinate of the other corner
    /// @param l The color of the line to draw with.
    /// @param f The color to fill the rectangle with.
    void drawFilledRect(uint x0, uint y0, uint x1, uint y1, uint8_t l,
                        uint8_t f);

    /// @brief Scrolls the canvas up by one row, filling the empty row with
    /// the background color.
    void scrollUp();

    /// @brief Scrolls the canvas up by n rows, filling the empty rows with the
    /// given color.
    /// @param n the number of rolls to scroll up by.
    /// @param f The color to fill the emptied rows.
    void scrollUp(int n, uint8_t f);

    /// @brief Scroll the canvas down by one row, filling in the empty row with
    /// the background color.
    void scrollDown();

    /// @brief Scrolls the canvas down by n rows, filling in the empty row with
    /// the given color.
    /// @param n
    /// @param f
    void scrollDown(int n, uint8_t f);

    void scrollLeft() { scrollLeft(1, colorMap->getBackgroundIndex()); };

    void scrollLeft(int n, uint8_t f);

    void scrollRight() { scrollRight(1, colorMap->getBackgroundIndex()); };

    void scrollRight(int n, uint8_t f);

    /// @brief Copies data from row src into row dst
    /// @param src The source of data
    /// @param dst The destination of data
    void copyRow(int src, int dst);

    void copyColumn(int src, int dst);

    void rotateRight();

    void rotateLeft();

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

    /// @brief Mirrors the right side of the canvas onto the left through the
    /// middle column.
    void mirrorRightToLeft();

    /// @brief Mirrors the right side of the canvas onto the left, where we
    /// mirror on column c.
    /// @param c the column to mirror on.
    void mirrorRightToLeft(int c);

    /// @brief Mirror the top of the canvas onto the bottom, reflecting through
    /// the center.
    void mirrorTopToBottom();

    /// @brief Mirrors the top of the canvas onto the bottom, reflecting through
    /// the given row.
    /// @param r the row we'll reflect though
    void mirrorTopToBottom(int r);

    /// @brief  Mirrors the bottom of the canvas onto the top, reflecting
    /// through the center.
    void mirrorBottomToTop();

    /// @brief mirrors the bottom of the canvas onto the top, reflecting through
    /// the given row.
    /// @param r the row we'll reflect through.
    void mirrorBottomToTop(int r);

    /// @brief Shifts a rectangle of pixels to the left by n pixels.
    void shiftLeft(int x, int y, uint w, uint h, int n);

    /// @brief Shifts a rectangle of pixels to the right by n pixels.
    void shiftRight(int x, int y, uint w, uint h, int n);

    /// @brief Shifts a rectangle of pixels to the left by n pixels.
    void shiftUp(int x, int y, uint w, uint h, int n);

    /// @brief Shifts a rectangle of pixels to the left by n pixels.
    void shiftDown(int x, int y, uint w, uint h, int n);

    /// @brief Gets the position in our data array for a given x and y. Just
    /// here so friends can figure it out
    /// @param x the x co-ordinate
    /// @param y the y co-ordinate
    /// @return the index into our data array for this x and y.
    uint getPos(uint x, uint y);

    /// @brief Gets a pointer into the underlying data array for the given x,y
    /// coordinate. Use carefully!
    /// @param x the x coordinate
    /// @param y the y coordinate
    /// @return a pointer into the data array, or NULL if the coords are weird.
    uint8_t* getDataPointer(uint x, uint y);

    /// @brief Clears the canvas.
    void clear();

    /// @brief Clears a region of the canvas
    /// @param x The x coordinate to start clearing at
    /// @param y The y coordinate to start clearing at
    /// @param w The width of the region
    /// @param h The height of the region
    void clear(uint x, uint y, uint w, uint h);

    /// @brief Clears a row of the canvas.
    /// @param row the row to clear.
    void clearRow(int row);

    /// @brief Clears a column of the canvas.
    /// @param column the column to clear.
    void clearColumn(int column);

    /// @brief Shows the data on the canvas in the view provided by the strips.
    void show();

    void debugPrint();

    void printRect(int x, int y, int w, int h);

    StopWatch* getStats();
};

#endif