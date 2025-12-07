#ifndef RANDOMANIMATOR_H
#define RANDOMANIMATOR_H

#pragma once

#include <stdint.h>

#include <vector>

#include "Animator.h"
#include "Canvas.h"

/// @brief An animation that pulls an animation randomly from a list. Will cycle
/// through all the animations before re-randomizing the order for the next
/// iteration through.
class RandomAnimator : public Animator {

   public:
    RandomAnimator(Canvas *canvas) : Animator(canvas) {};
    RandomAnimator(Canvas *canvas, int fps) : Animator(canvas, fps) {}
    void animationChanged() override;
};
#endif