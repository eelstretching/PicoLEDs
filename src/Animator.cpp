#include "Animator.h"

#include "TimedAnimation.h"

Animator::~Animator() {
    for (auto a : owned) {
        delete a;
    }
}

void Animator::add(Animation* a) { animations.push_back(a); }

void Animator::addTimed(Animation* a, int durationMS) {
    TimedAnimation* ta = new TimedAnimation(a, durationMS);
    owned.push_back(ta);
    animations.push_back(ta);
}

void Animator::setFPS(int fps) {
    this->fps = fps > 0 ? fps : kDefaultFPS;
    //
    // There's a million microseconds in a second.
    usPerFrame = 1000000 / this->fps;
}

void Animator::init() {
    pos = 0;
    animationChanged();
    if (animations.empty() || animations[pos] == nullptr) {
        return;
    }
    uint64_t now = clockUs();
    startAnimation(animations[pos], now - now % usPerFrame);
}

void Animator::startAnimation(Animation* a, uint64_t at) {
    a->startClock(at);
    a->init();
    //
    // Owed exactly one step on its first frame.
    int sfps = a->getFPS();
    uint32_t period = sfps > 0 ? 1000000 / sfps : 0;
    stepOwedUs = period > usPerFrame ? period - usPerFrame : 0;
}

int Animator::stepsDue(Animation* a) {
    int sfps = a->getFPS();
    if (sfps <= 0) {
        return 1;
    }
    uint32_t period = 1000000 / sfps;
    stepOwedUs += usPerFrame;
    int n = stepOwedUs / period;
    stepOwedUs -= n * period;
    return n > kMaxStepsPerFrame ? kMaxStepsPerFrame : n;
}

void Animator::printStats() {
    uint64_t runTime = time_us_64() - startTime;
    printf(
        "%d frames run, running %.1fs %.2f us/frame at %d fps %.2f us/step, %.2f us/dst %.2f "
        "us/show %.2f us/frame %d missed frames %d refreshes\n",
        getFrameCount(), runTime / 1000000.0, 
        getUsPerFrame(), getFPS(),
        getAverageStepTimeUS(), 
        getAverageDataPrepTimeUS(),
        getAverageShowTimeUS(), getAverageFrameTimeUS(),
        getMissedFrames(), getRefreshCount());
}

bool Animator::step() {
    if (animations.size() == 0) {
        return false;
    }
    uint64_t now = clockUs();
    frameTime = now - now % usPerFrame;

    //
    // Step the current animation as many times as it's due this frame (often
    // once, maybe none or a few for animations with their own step rate). If
    // it finishes, get ready for the next one in the list, looping around to
    // the start of the list if we've reached the end.
    frameWatch.start();
    stepWatch.start();
    bool changed = false;
    Animation* a = animations[pos];
    int n = stepsDue(a);
    for (int i = 0; i < n; i++) {
        a->setFrameTime(frameTime);
        if (!a->step()) {
            a->finish();
            pos = (pos + 1) % animations.size();
            changed = true;
            break;
        }
    }
    stepWatch.finish();
    showWatch.start();
    canvas->show();
    showWatch.finish();
    frameWatch.finish();
    frameCount++;

    //
    // We'll do this out here to avoid flashes due to color map changes in
    // init(). The new animation's first step is on the next frame.
    if (changed) {
        animationChanged();
        startAnimation(animations[pos], frameTime + usPerFrame);
    }

    waitForNextFrame();
    return true;
}

void Animator::waitForNextFrame() {
    uint64_t next = frameTime + usPerFrame;
    uint64_t now = clockUs();
    if (now >= next) {
        //
        // Late. The next step() starts right away, on the frame we're in now.
        missedFrames++;
        return;
    }
    if (next - now > usPerFrame) {
        //
        // The clock jumped backwards (a shared clock stepping to correct
        // itself). Line up with the new clock's next frame.
        next = now - now % usPerFrame + usPerFrame;
    }
    //
    // If we're dithering, then we want to spend the time until the next frame
    // re-sending this frame, since each send rounds a little differently and
    // it's the average over sends that gets the in-between levels right. We'll
    // stop when another send wouldn't finish before the next frame is due.
    if (canvas->getRenderer()->getDithering()) {
        uint64_t last = 0;
        while (now + last < next) {
            canvas->show();
            refreshCount++;
            uint64_t after = clockUs();
            last = after - now;
            now = after;
        }
    }
    now = clockUs();
    if (now < next) {
        sleep_us(next - now);
    }
}
