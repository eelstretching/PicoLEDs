// LinesFill and Firework both define a FillState, so EffectLifecycle makes its
// LinesFill here, away from the fireworks.

#include "LinesFill.h"

Animation* makeLinesFill(Canvas* canvas, ColorMap* colorMap, uint8_t* colors) {
    return new LinesFill(canvas, colorMap, 5, colors, UP, 1);
}
