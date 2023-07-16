#ifndef FIREWORK_H
#define FIREWORK_H

#pragma once

#include "Animation.h"
#include "Canvas.h"

#define EXPLOSION_DIVISOR 4

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
    PAUSE,
    RISING,
    START_EXPLOSION,
    EXPLODING,
    RESET,
};

class Firework : public Animation {
    /// @brief The row that we'll be animating
    uint row;

    /// @brief The number of frames to pause before launching. Randomly chose
    /// after each explotion to space things out a little better.
    uint8_t pauseTime;

    /// @brief The rising flare, which has a head and a tail. The head is at
    /// position 0.
    Spark *flare;

    /// @brief The size of the trail for a rising flare.
    uint flareSize = 3;

    /// @brief The sparks that make up the explosion of the flare.
    Spark *explosion;

    /// @brief The size of the explosion that we're using this time around.
    uint explosionSize;

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
    float gravity = 0.005;

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

    /// @brief Generate a random flare position so that we can just do the
    /// explosion.
    void randomFlare();

    /// @brief Set up for a flare that starts right in the middle of the row.
    void halfFlare();

    /// @brief Gets the current state of the firework.
    /// @return The state.
    State getState() { return state; };

    /// @brief Sets the maximum number of steps that an explosion will take. You
    /// can use this to limit the time for an explosion.
    /// @param maxExplosionSteps The number of steps to take, i.e., the number
    /// of frames. You can increase or decrease based on your fps to get a
    /// certain time.
    void setMaxExplosionSteps(uint maxExplosionSteps) {
        this->maxExplosionSteps = maxExplosionSteps;
    };

    /// @brief Sets the current state of the firework.
    /// @param state The state to use.
    void setState(State state) { this->state = state; };

    /// @brief Initialize our explosion.
    void startExplosion();

    /// @brief The flare explodes.
    void explode();

    bool step();
};
#endif