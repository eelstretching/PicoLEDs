#ifndef RANDOMANIMATION_H
#define RANDOMANIMATION_H

#pragma once

#include <vector>

#include "Animation.h"
#include "Canvas.h"

/// @brief A wipe that randomly chooses which wipe to use from a list.
class RandomAnimation : public Animation {

    protected:
    std::vector<Animation*> animations;
    Animation *curr;

    public:
    RandomAnimation(Canvas *canvas, ColorMap *colorMap) : Animation(canvas, colorMap) {
        curr = NULL;
    };
    void add(Animation *animation);
    void addTimed(Animation *animation,  int durationMS);
    Animation* getCurr();
    void init() override;
    bool step() override;
    void finish() override;
    int getFPSNeeded() override;

};
#endif