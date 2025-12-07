#include "RandomAnimation.h"

#include <TimedAnimation.h>

#include <algorithm>
#include <random>

#include "math8.h"

void RandomAnimation::add(Animation* animation) {
    animations.push_back(animation);
}

void RandomAnimation::addTimed(Animation* animation, int durationMS) {
    TimedAnimation* ta = new TimedAnimation(animation, durationMS);
    animations.push_back(ta);
}

void RandomAnimation::shuffle() {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(animations.begin(), animations.end(), g);
}

Animation* RandomAnimation::getCurr() {
    if (currp >= animations.size()) {
        printf("Shuffling animations...\n");
        currp = 0;
        shuffle();
    }
    curr = animations[currp];
    if (initNeeded) {
        printf("Initializing %s\n", curr->getName());
        curr->init();
        initNeeded = false;
    }
    return curr;
}

void RandomAnimation::init() {
    printf("Init for %d animations\n", animations.size());
    initNeeded = true;
    currp = animations.size() + 1;
    getCurr();
}

bool RandomAnimation::step() {
    getCurr();
    if (!curr->step()) {
        printf("Finishing %s\n", curr->getName());
        //
        // Move onto the next animation in the list.
        currp++;
        initNeeded = true;
    }
    return true;
}

void RandomAnimation::finish() {
    getCurr();
    curr->finish();
}

int RandomAnimation::getFPSNeeded() {
    getCurr();
    return curr->getFPSNeeded();
}
