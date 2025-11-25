#include "Animation.h"

Animation::Animation() { canvas = NULL; colorMap = NULL; }

Animation::Animation(Canvas* canvas, ColorMap *colorMap) : canvas(canvas), colorMap(colorMap) {}

