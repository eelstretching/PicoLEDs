#ifndef TREELIGHT_H
#define TREELIGHT_H

#pragma once

#include <stdint.h>
#include "Animation.h"

/// @brief A light on a tree that fades in and out.
class TreeLight : public Animation {
    /// @brief the canvas that we'll render our light onto.
    Canvas *canvas;

    uint x;
    uint y;
    uint size;

    /// @brief The color of the light.
    RGB color;

    uint8_t brightness = 255;

    /// @brief Is the light fading in or out?
    /// @details If true, the light is fading in, if false, it's fading out.
    /// This is used to determine how to change the color.
    bool fadingIn = false;

   public:
    TreeLight(Canvas *canvas, uint x, uint y, uint size, RGB color, uint row);
    ~TreeLight();

    bool step();
};
#endif