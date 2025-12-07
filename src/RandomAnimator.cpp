#include "RandomAnimator.h"

#include <TimedAnimation.h>

#include <algorithm>
#include <random>

#include "math8.h"

void RandomAnimator::animationChanged() {
    //
    // If we wrapped around to the start of the list, then let's shuffle the list.
    if (pos == 0) {
        printf("Shuffling list of %d animations\n", animations.size());
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(animations.begin(), animations.end(), g);
    }
    printf("Animation changed to %s\n", animations[pos]->getName());
}

