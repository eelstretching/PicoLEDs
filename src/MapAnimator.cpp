#include "MapAnimator.h"

void MapAnimator::add(int key, Animation* a) {
    keys.push_back(key);
    Animator::add(a);
}

void MapAnimator::set(int key) {
    for (int i = 0; i < keys.size(); i++) {
        if (keys[i] == key) {
            pos = i;
            curr = i;
            uint64_t now = clockUs();
            startAnimation(animations[pos], now - now % usPerFrame + usPerFrame);
            return;
        }
    }
}

void MapAnimator::animationChanged() {
    //
    // This animation will keep running over and over until we change it. So, if
    // the animation changed, then we need to reset the position back to the
    // current animation.
    pos = curr;
}
