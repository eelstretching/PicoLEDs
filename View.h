#ifndef VIEW_H
#define VIEW_H
#pragma once

#include <vector>

#include "Strip.h"
#include "pico/stdlib.h"
#include "pico/types.h"

class Canvas;

enum Direction {
    FORWARDS,
    BACKWARDS,
};

class Row {
    friend class View;
    Strip *strip;
    uint start;
    uint n;
    Direction dir;

   public:
    Row(Strip *strip, uint start, uint n, Direction dir)
        : strip(strip), start(start), n(n), dir(dir) {}
};

/// @brief A view that we can use when rendering a canvas. A view also has a
/// height and width. The consituents of a View are strips upon which the view
/// is rendered. A view is composed of Rows, and each Row may be a part of or
/// a whole strip.
///
/// As with a Canvas, a View's origin is at the bottom left (i.e., Cartesian
/// co-ordinates) and strips are filled from the origin up. A View presumes that
/// if you add a Strip that is wider than the view, that a) it is folded over
/// into multiple rows and b) that the number of pixels in the Strip is an
/// integer multiple of the width of the View.
class View {

    /// @brief The width of this view. 
    uint width;
    
    /// @brief The canvas that we're a view of.
    Canvas *canvas;

    /// @brief The rows that make up this view, which are distributed across the
    /// strips that were added.
    std::vector<Row> rows;

   public:
    View(uint width) : width(width) {}

    /// @brief Sets the canvas that we are a view of.
    /// @param canvas The canvas we're a view of
    /// @param x The x position of the origin of this view on the canvas
    /// @param y The y position of the origin of this view on the canvas
    void setCanvas(Canvas *canvas);

    /// @brief Adds a strip of pixels to this view.
    /// @param strip
    void add(Strip strip);

    ///
    /// @brief Renders this view of the given canvas.
    void render();
};

#endif