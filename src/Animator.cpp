#include "Animator.h"

#include "TimedAnimation.h"

Animator::Animator(Canvas* canvas, ColorMap* colorMap)
    : Animation(canvas, colorMap) {
    //
    // We'll do 30 FPS by default.
    setFPS(30);
}

Animator::Animator(Canvas* canvas, ColorMap* colorMap, int fps)
    : Animator(canvas, colorMap) {
    setFPS(fps);
}

void Animator::add(Animation* a) { animations.push_back(a); }

void Animator::addTimed(Animation* a, int durationMS) {
    TimedAnimation* ta = new TimedAnimation(a, durationMS);
    animations.push_back(ta);
}

void Animator::setFPS(int fps) {
    this->fps = fps;
    //
    // There's a million microseconds in a second.
    usPerFrame = 1e6 / fps;
}

void Animator::init() {
    pos = 0;
    Animation* f = animations[pos];
    setFPS(f->getFPSNeeded());
    f->init();
}

void Animator::printStats() {
    printf(
        "%d frames run, %.2f us/frame at %d fps %.2f us/step, %.2f "
        "us/show %.2f us/frame %d missed frames\n",
        getFrameCount(), getUsPerFrame(), getFPSNeeded(),
        getAverageStepTimeUS(), getAverageShowTimeUS(), getAverageFrameTimeUS(),
        getMissedFrames());
}

bool Animator::step() {
    if (animations.size() == 0) {
        return false;
    }
    //
    // Take a step on the current animation. If that animation finishes, then
    // get ready for the next one in the list, looping around to the start of
    // the list if we've reached the end.
    frameWatch.start();
    stepWatch.start();
    bool changed = false;
    if (!animations[pos]->step()) {
        animations[pos]->finish();
        pos = (pos + 1) % animations.size();
        changed = true;
    }
    stepWatch.finish();
    showWatch.start();
    canvas->show();
    showWatch.finish();
    frameWatch.finish();
    frameCount++;

    //
    // We'll do this out here to avoid flashes due to color map changes in
    // init().
    if (changed) {
        animations[pos]->init();
        setFPS(animations[pos]->getFPSNeeded());
    }

    //
    // Sleep until it's time for the next frame. We're not trying to be
    // super-precise here.
    uint64_t lus = frameWatch.getLastTime();
    if (lus < usPerFrame) {
        sleep_us(usPerFrame - lus);
    } else {
        missedFrames++;
    }
    return true;
}
