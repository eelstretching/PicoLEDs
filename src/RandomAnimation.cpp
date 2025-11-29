#include "RandomAnimation.h"

#include <TimedAnimation.h>

#include "math8.h"

void RandomAnimation::add(Animation* animation) {
    animations.push_back(animation);
}

void RandomAnimation::addTimed(Animation* animation, int durationMS) {
    TimedAnimation* ta = new TimedAnimation(animation, durationMS);
    printf("%d TA name: %s\n", animations.size(), ta->getName());
    animations.push_back(ta);
}

Animation* RandomAnimation::getCurr() {
    if (curr == nullptr) {
        int p = random8(0, animations.size());
        curr = animations[p];
        printf("Choosing random animation: %d %s\n", p, curr->getName());
    }
    return curr;
}

void RandomAnimation::init() { 
    getCurr();
    curr->init(); }

bool RandomAnimation::step() {
    getCurr();
    if (!curr->step()) {
        curr = nullptr;
        return false;
    }
    return true;
}

void RandomAnimation::finish() { getCurr(); curr->finish(); }

int RandomAnimation::getFPSNeeded() { getCurr(); return curr->getFPSNeeded(); }
