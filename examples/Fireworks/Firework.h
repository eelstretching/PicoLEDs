#ifndef FIREWORK_H
#define FIREWORK_H

#pragma once

#include "Animation.h"
#include "Canvas.h"

class Spark {
    friend class Firework;

    /// @brief The position of the spark in the row of pixels.
    float pos;
    /// @brief The velocity of the spark.
    float vel;
    /// @brief The value for an HSV color for this spark.
    float val;

   public:
    Spark() {
        pos = 0;
        vel = 0;
        val = 0;
    };
};

enum State {
    RISING,
    START_EXPLOSION,
    EXPLODING,
    RESET,
};

class Firework : Animation {
    /// @brief the canvas that we'll render our fire onto.
    Canvas *canvas;

    /// @brief The row that we'll be animating
    uint row;

    /// @brief The rising flare, which has a head and a tail. The head is at
    /// position 0.
    Spark *flare;

    /// @brief The size of the trail for a rising flare.
    uint flareSize = 6;

    /// @brief The sparks that make up the explosion of the flare.
    Spark *explosion;

    /// @brief How many sparks we'll generate when exploding.
    uint numSparks;

    /// @brief How many steps we've taken in the explosion.
    uint explosionSteps;

    /// @brief The maximum number of explosion steps that we want to take, in
    /// order to keep things moving along. Three seconds at 30 frames per
    /// second.
    uint maxExplosionSteps = 90;

    /// @brief The state of the animation.
    State state = RESET;

    /// @brief Our good old friend gravity. Gravity will be subtracted from
    /// spark velocity as time increases.
    float gravity = 0.004;

    /// @brief Gravity for sparks that are falling from an explosion.
    float dyingGravity;

    /// @brief Colors that we'll use to "time" the explosion. As the sparks fade
    /// out, we'll reset.
    float c1, c2;

   public:
    /// @brief Constructor for a firework
    /// @param canvas The canvas where we'll be drawing
    /// @param row The row we'll be drawing in
    Firework(Canvas *canvas, uint row);

    /// @brief Resets the simulation when the explosion is done.
    void reset();

    /// @brief The flare rises, trailing a number of sparks.
    void rise();

    /// @brief Initialize our explosion.
    void startExplosion();

    /// @brief The flare explodes.
    void explode();

    void step();

    
};
#endif