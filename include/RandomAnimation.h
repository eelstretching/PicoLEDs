#ifndef RANDOMWIPE_H
#define RANDOMWIPE_H

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
    RandomAnimation(Canvas *canvas) : Animation(canvas) {};
    void add(Animation *animation);
    void init();
    bool step();
    void finish();

};
#endif