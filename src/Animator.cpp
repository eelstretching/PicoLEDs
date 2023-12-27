#include "Animator.h"

Animator::Animator(Canvas* canvas) : Animation(canvas) {
    //
    // We'll do 30 FPS by default.
    setFPS(30);
}

Animator::Animator(Canvas* canvas, int fps) : Animator(canvas) { setFPS(fps); }

void Animator::add(Animation* a) { animations.push_back(a); }

void Animator::init() {
    pos = 0;
    Animation *f = animations[pos];
    setFPS(f->getFPSNeeded());
    f->init();
}

bool Animator::step() {
    if (animations.size() == 0) {
        return false;
    }
    //
    // Take a step on the current animation. If that animation finishes, then
    // get ready for the next one in the list, looping around to the start of
    // the list if we've reached the end.
    aw.start();
    if (!animations[pos]->step()) {
        animations[pos]->finish();
        pos = (pos + 1) % animations.size();
        setFPS(animations[pos]->getFPSNeeded());
        animations[pos]->init();
    }
    sw.start();
    canvas->show();
    sw.finish();
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
