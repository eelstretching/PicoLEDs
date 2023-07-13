#include "RandomAnimation.h"
#include "math8.h"

void RandomAnimation::add(Animation* animation) {
    animations.push_back(animation);
}

void RandomAnimation::init() {
    int p = random8(0, animations.size());
    curr = animations[p];
    curr->init();
}

bool RandomAnimation::step() { 
    return curr->step();
 }

void RandomAnimation::finish() {
    curr->finish();
}
