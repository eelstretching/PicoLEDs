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
        //
        // Seed from the Pico's own random number generator. That's real
        // randomness on the hardware, and the simulator's --seed controls it,
        // where std::random_device is neither.
        std::mt19937 g(get_rand_32());
        std::shuffle(animations.begin(), animations.end(), g);
    }
    printf("Animation changed to %s\n", animations[pos]->getName());
}

