#ifndef RANDOMANIMATION_H
#define RANDOMANIMATION_H

#pragma once

#include <vector>

#include "Animation.h"
#include "Canvas.h"

/// @brief An animation that pulls an animation randomly from a list. Will cycle through
/// all the animations before re-randomizing the order for the next iteration through.
class RandomAnimation : public Animation {

    protected:
    std::vector<Animation*> animations;
    Animation *curr;
    uint8_t currp;
    bool initNeeded;

    public:
    RandomAnimation(Canvas *canvas, ColorMap *colorMap) : Animation(canvas, colorMap) {
        curr = nullptr;
        currp = 0;
    };
    void add(Animation *animation);
    void addTimed(Animation *animation,  int durationMS);
    void shuffle();
    Animation* getCurr();
    void init() override;
    bool step() override;
    void finish() override;
    int getFPSNeeded() override;

};
#endif