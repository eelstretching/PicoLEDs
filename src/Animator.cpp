#include "Animator.h"

Animator::Animator(Canvas* canvas) : Animation(canvas) {
    //
    // We'll do 30 FPS by default.
    setFPS(30);
    animations = NULL;
    last = NULL;
}

Animator::Animator(Canvas* canvas, int fps) : Animator(canvas) { setFPS(fps); }

void Animator::add(Animation* a) {
    a->next = NULL;
    if (animations == NULL) {
        animations = a;
        last = a;
    } else {
        last->next = a;
        last = a;
    }
}

void Animator::init() {
    curr = animations;
}

bool Animator::step() {
    if (curr == NULL) {
        return false;
    }
    //
    // Take a step on the current animation. If that animation finishes, then
    // get ready for the next one in the list, looping around to the start of
    // the list if we've reached the end.
    aw.start();
    if (!curr->step()) {
        curr = curr->next;
        if (curr == NULL) {
            curr = animations;
        }
        curr->init();
    }
    canvas->show();
    aw.finish();

    //
    // Sleep until it's time for the next frame. We're not trying to be
    // super-precise here.
    uint64_t lms = aw.getLastTimeMS();
    if (lms < msPerFrame) {
        sleep_ms(msPerFrame - lms);
    } else {
        missedFrames++;
    }
    return true;
}

