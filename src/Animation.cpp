#include "Animation.h"

Animation::Animation() {
    canvas = NULL;
    next = NULL;
}

Animation::Animation(Canvas* canvas) : canvas(canvas) {
    next = NULL;
}
