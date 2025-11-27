#ifndef ICICLES_H
#define ICICLES_H
#pragma once

#include "Animation.h"
#include "Canvas.h"
#include "Direction.h"
#include "colorutils.h"

#define MAX_SPEED 3.0
enum DripState {
    DRIPPING,
    FALLING,
};

class Drip : public Animation {
    friend class Icicle;

    uint pos;

    /// @brief The position of the drip.
    int dp;

    /// @brief The length of the icicle the drip is coming from.
    int length;

    /// @brief The length of the tail of the drip while it's falling.
    int tailLen;

    /// @brief The direction the drip is moving.
    Direction direction;

    DripState state;

    float speed;

   public:
    /// @brief Constructor for a drip animation.
    /// @param canvas The canvas where we'll be drawing.
    /// @param pos The position (row or column) to draw in.
    /// @param length The length of the drip.
    /// @param direction The direction the drip is moving.
    Drip(Canvas* canvas, ColorMap* colorMap, uint pos, uint length,
         uint tailLen, Direction direction);

    /// @brief Initializes the animation.
    void init() override;

    void setPos(uint pos);

    /// @brief Steps the animation forward one frame.
    /// @return True if the animation is still running, false if it's done.
    bool step() override;
};

class Icicle : public Animation {
    friend class Icicles;

    /// @brief The column or row that we'll be animating
    uint pos;

    /// @brief The length of the icicle.
    uint length;

    /// @brief The direction the icicle is facing, and the direction of gravity.
    Direction direction = LEFT;

    ColorMap* colorMap;

    Drip* drip;

   public:
    /// @brief Constructor for an icicle animation.
    /// @param canvas The canvas where we'll be drawing.
    /// @param pos The column to draw in.
    /// @param length The length of the icicle.
    /// @param color The brightest color of the icicle.
    Icicle(Canvas* canvas, ColorMap* colorMap, uint pos, uint length,
           Direction direction, RGB color = RGB::White);

    void setPos(uint pos);

    void init() override;

    /// @brief Steps the animation forward one frame.
    /// @return True if the animation is still running, false if it's done.
    bool step() override;
};

class Icicles : public Animation {
    /// @brief The number of icicles to draw.
    uint numIcicles;

    /// @brief The array of icicles.
    Icicle** icicles;

   public:
    /// @brief Constructor for an icicles animation.
    /// @param canvas The canvas where we'll be drawing.
    /// @param numIcicles The number of icicles to draw.
    /// @param length The base length of the icicles, we'll randomize a bit.
    /// @param color The brightest color of the icicles.
    Icicles(Canvas* canvas, ColorMap* colorMap, uint numIcicles, uint length,
            RGB color);

    /// @brief Destructor for the icicles animation.
    ~Icicles();

    int getFPSNeeded() override { return 30; };

    void init() override;

    bool step() override;
};
#endif